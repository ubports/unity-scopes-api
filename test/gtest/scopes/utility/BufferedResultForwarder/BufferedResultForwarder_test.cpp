/*
 * Copyright (C) 2014 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Pawel Stolowski <pawel.stolowski@canonical.com>
 */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <gtest/gtest.h>
#pragma GCC diagnostic pop

#include <unity/scopes/utility/BufferedResultForwarder.h>
#include <unity/scopes/CategoryRenderer.h>
#include <unity/scopes/testing/MockSearchReply.h>
#include <unity/scopes/testing/Category.h>
#include <unity/UnityExceptions.h>

using namespace unity::scopes;
using namespace unity::scopes::utility;
using namespace unity::scopes::internal;
using namespace ::testing;

class SearchReceiver : public BufferedResultForwarder
{
    public:
        SearchReceiver(unity::scopes::SearchReplyProxy const& upstream)
            : BufferedResultForwarder(upstream)
        {
        }

        SearchReceiver(unity::scopes::SearchReplyProxy const& upstream, BufferedResultForwarder::SPtr const& next_forwarder)
            : BufferedResultForwarder(upstream, next_forwarder)
        {
        }

        void push(CategorisedResult result) override
        {
            upstream()->push(result);
            set_ready();
        }
};

// a receiver that never calls 'set_ready'; results will get flushed when SearchListenerBase::finished() gets called.
class SearchReceiverWithoutSetReady : public BufferedResultForwarder
{
    public:
        SearchReceiverWithoutSetReady(unity::scopes::SearchReplyProxy const& upstream)
            : BufferedResultForwarder(upstream)
        {
        }

        SearchReceiverWithoutSetReady(unity::scopes::SearchReplyProxy const& upstream, BufferedResultForwarder::SPtr const& next_forwarder)
            : BufferedResultForwarder(upstream, next_forwarder)
        {
        }

        void push(CategorisedResult result) override
        {
            // never call set_ready()
            upstream()->push(result);
        }
};

MATCHER_P2(ResultProp, prop, value, "")
{
    if (arg.contains(prop)) {
        *result_listener << "result[" << prop << "] is " << arg[prop].serialize_json();
    } else {
        *result_listener << "result[" << prop << "] is not set";
    }
    return arg.contains(prop) && arg[prop] == unity::scopes::Variant(value);
}

void push_results(Category::SCPtr const& cat, std::string const& uri, std::string const& title, int count, SearchListenerBase::SPtr const& search_listener)
{
    for (int i = 0; i<count; i++)
    {
        CategorisedResult res(cat);
        res.set_uri(uri);
        res.set_title(title);

        search_listener->push(res);
    }
}

TEST(BufferedResultForwarder, basic)
{
    const unity::scopes::CategoryRenderer renderer{};

    NiceMock<unity::scopes::testing::MockSearchReply> reply;
    unity::scopes::SearchReplyProxy upstream
    {
        &reply, [](unity::scopes::SearchReply*) {}
    };

    // results from fwd1 first, then fwd2, then fwd3
    auto fwd3 = std::make_shared<SearchReceiver>(upstream);
    auto fwd2 = std::make_shared<SearchReceiver>(upstream, fwd3);
    auto fwd1 = std::make_shared<SearchReceiver>(upstream, fwd2);

    SearchListenerBase::SPtr base3 = fwd3;
    SearchListenerBase::SPtr base2 = fwd2;
    SearchListenerBase::SPtr base1 = fwd1;

    EXPECT_CALL(reply, push(Matcher<unity::scopes::CategorisedResult const&>(ResultProp("title", "scope1")))).Times(3);
    EXPECT_CALL(reply, push(Matcher<unity::scopes::CategorisedResult const&>(ResultProp("title", "scope2")))).Times(10);
    EXPECT_CALL(reply, push(Matcher<unity::scopes::CategorisedResult const&>(ResultProp("title", "scope3")))).Times(1);

    // results from scope2 arrive first
    {
        EXPECT_FALSE(fwd2->is_ready());

        Category::SCPtr cat = std::make_shared<unity::scopes::testing::Category>("scope2cat", "Scope2", "", renderer);
        base2->push(cat);
        push_results(cat, "scope2", "scope2", 10, base2);

        EXPECT_TRUE(fwd2->is_ready());
    }

    // results coming from scope1
    {

        EXPECT_FALSE(fwd1->is_ready());

        Category::SCPtr cat = std::make_shared<unity::scopes::testing::Category>("scope1cat", "Scope1", "", renderer);
        base1->push(cat);
        push_results(cat, "scope1", "scope1", 3, base1);

        EXPECT_TRUE(fwd1->is_ready());
    }

    // result from scope3
    {
        EXPECT_FALSE(fwd3->is_ready());

        Category::SCPtr cat = std::make_shared<unity::scopes::testing::Category>("scope3cat", "Scope3", "", renderer);
        base1->push(cat);
        push_results(cat, "scope3", "scope3", 1, base3);

        EXPECT_TRUE(fwd3->is_ready());
    }

    CompletionDetails status(CompletionDetails::CompletionStatus::OK);
    base3->finished(status);
    base1->finished(status);
    base2->finished(status);
}

TEST(BufferedResultForwarder, flush_on_finished)
{
    const unity::scopes::CategoryRenderer renderer{};

    NiceMock<unity::scopes::testing::MockSearchReply> reply;
    unity::scopes::SearchReplyProxy upstream
    {
        &reply, [](unity::scopes::SearchReply*) {}
    };

    // results from fwd1 first, then fwd2
    auto fwd2 = std::make_shared<SearchReceiver>(upstream);
    auto fwd1 = std::make_shared<SearchReceiverWithoutSetReady>(upstream, fwd2);

    SearchListenerBase::SPtr base2 = fwd2;
    SearchListenerBase::SPtr base1 = fwd1;

    EXPECT_CALL(reply, push(Matcher<unity::scopes::CategorisedResult const&>(ResultProp("title", "scope1")))).Times(2);
    EXPECT_CALL(reply, push(Matcher<unity::scopes::CategorisedResult const&>(ResultProp("title", "scope2")))).Times(2);

    // results from scope2 arrive first
    {
        Category::SCPtr cat = std::make_shared<unity::scopes::testing::Category>("scope2catA", "Scope2", "", renderer);
        base2->push(cat);
        push_results(cat, "scope2", "scope2", 2, base2);

        EXPECT_TRUE(fwd2->is_ready());
    }

    // results coming from scope1
    {
        Category::SCPtr cat = std::make_shared<unity::scopes::testing::Category>("scope1catA", "Scope1", "", renderer);
        base1->push(cat);
        push_results(cat, "scope1", "scope1", 2, base1);

        EXPECT_FALSE(fwd1->is_ready());
    }

    CompletionDetails status(CompletionDetails::CompletionStatus::OK);
    base1->finished(status);
    EXPECT_TRUE(fwd1->is_ready());
    base2->finished(status);
}

TEST(BufferedResultForwarder, flush_on_finished_reversed_order)
{
    const unity::scopes::CategoryRenderer renderer{};

    NiceMock<unity::scopes::testing::MockSearchReply> reply;
    unity::scopes::SearchReplyProxy upstream
    {
        &reply, [](unity::scopes::SearchReply*) {}
    };

    // results from fwd1 first, then fwd2
    auto fwd2 = std::make_shared<SearchReceiver>(upstream);
    auto fwd1 = std::make_shared<SearchReceiverWithoutSetReady>(upstream, fwd2);

    SearchListenerBase::SPtr base2 = fwd2;
    SearchListenerBase::SPtr base1 = fwd1;

    EXPECT_CALL(reply, push(Matcher<unity::scopes::CategorisedResult const&>(ResultProp("title", "scope1")))).Times(2);
    EXPECT_CALL(reply, push(Matcher<unity::scopes::CategorisedResult const&>(ResultProp("title", "scope2")))).Times(2);

    // results from scope2 arrive first
    {
        Category::SCPtr cat = std::make_shared<unity::scopes::testing::Category>("scope2catA", "Scope2", "", renderer);
        base2->push(cat);
        push_results(cat, "scope2", "scope2", 2, base2);
    }

    // results coming from scope1
    {
        Category::SCPtr cat = std::make_shared<unity::scopes::testing::Category>("scope1catA", "Scope1", "", renderer);
        base1->push(cat);
        push_results(cat, "scope1", "scope1", 2, base1);
    }

    CompletionDetails status(CompletionDetails::CompletionStatus::OK);

    // the order in which finished() is called shouldn't matter - buffers are flushed in the requested order
    base2->finished(status);
    base1->finished(status);
}

TEST(BufferedResultForwarder, flush_on_finished_for_all_scopes)
{
    const unity::scopes::CategoryRenderer renderer{};

    NiceMock<unity::scopes::testing::MockSearchReply> reply;
    unity::scopes::SearchReplyProxy upstream
    {
        &reply, [](unity::scopes::SearchReply*) {}
    };

    // results from fwd1 first, then fwd2
    auto fwd2 = std::make_shared<SearchReceiverWithoutSetReady>(upstream);
    auto fwd1 = std::make_shared<SearchReceiverWithoutSetReady>(upstream, fwd2);

    SearchListenerBase::SPtr base2 = fwd2;
    SearchListenerBase::SPtr base1 = fwd1;

    EXPECT_CALL(reply, push(Matcher<unity::scopes::CategorisedResult const&>(ResultProp("title", "scope1")))).Times(2);
    EXPECT_CALL(reply, push(Matcher<unity::scopes::CategorisedResult const&>(ResultProp("title", "scope2")))).Times(2);

    // results from scope2 arrive first
    {
        Category::SCPtr cat = std::make_shared<unity::scopes::testing::Category>("scope2catA", "Scope2", "", renderer);
        base2->push(cat);
        push_results(cat, "scope2", "scope2", 2, base2);
        EXPECT_FALSE(fwd1->is_ready());
    }

    // results coming from scope1
    {
        Category::SCPtr cat = std::make_shared<unity::scopes::testing::Category>("scope1catA", "Scope1", "", renderer);
        base1->push(cat);
        push_results(cat, "scope1", "scope1", 2, base1);
        EXPECT_FALSE(fwd2->is_ready());
    }

    CompletionDetails status(CompletionDetails::CompletionStatus::OK);

    base2->finished(status);
    EXPECT_TRUE(fwd2->is_ready());
    EXPECT_FALSE(fwd1->is_ready());
    base1->finished(status);
    EXPECT_TRUE(fwd1->is_ready());
}

TEST(BufferedResultForwarder, no_results_from_first_scope)
{
    const unity::scopes::CategoryRenderer renderer{};

    NiceMock<unity::scopes::testing::MockSearchReply> reply;
    unity::scopes::SearchReplyProxy upstream
    {
        &reply, [](unity::scopes::SearchReply*) {}
    };

    // results from fwd1 first, then fwd2
    auto fwd2 = std::make_shared<SearchReceiver>(upstream);
    auto fwd1 = std::make_shared<SearchReceiverWithoutSetReady>(upstream, fwd2);

    SearchListenerBase::SPtr base2 = fwd2;
    SearchListenerBase::SPtr base1 = fwd1;

    EXPECT_CALL(reply, push(Matcher<unity::scopes::CategorisedResult const&>(ResultProp("title", "scope2")))).Times(2);

    // results from scope2 arrive first
    {
        Category::SCPtr cat = std::make_shared<unity::scopes::testing::Category>("scope2catA", "Scope2", "", renderer);
        base2->push(cat);
        push_results(cat, "scope2", "scope2", 2, base2);
    }

    CompletionDetails status(CompletionDetails::CompletionStatus::OK);
    base2->finished(status);
    base1->finished(status);
}

TEST(BufferedResultForwarder, exceptions)
{
    NiceMock<unity::scopes::testing::MockSearchReply> reply;
    unity::scopes::SearchReplyProxy upstream
    {
        &reply, [](unity::scopes::SearchReply*) {}
    };

    auto fwd3 = std::make_shared<SearchReceiver>(upstream);
    auto fwd2 = std::make_shared<SearchReceiver>(upstream, fwd3);
    EXPECT_THROW(std::make_shared<SearchReceiver>(upstream, fwd3), unity::LogicException);
}

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

#include <gtest/gtest.h>
#include <unity/scopes/utility/BufferedResultForwarder.h>
#include <unity/scopes/CategoryRenderer.h>
#include <unity/scopes/testing/MockSearchReply.h>
#include <unity/scopes/testing/Category.h>

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
            if (result.title() == "scope1")
            {
                set_ready();
            }
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

TEST(BufferedResultForwarder, basic)
{
    const unity::scopes::CategoryRenderer renderer{};

    NiceMock<unity::scopes::testing::MockSearchReply> reply;
    unity::scopes::SearchReplyProxy upstream
    {
        &reply, [](unity::scopes::SearchReply*) {}
    };

    // results from fwd1 first, then fwd2
    auto fwd2 = std::make_shared<SearchReceiver>(upstream);
    auto fwd1 = std::make_shared<SearchReceiver>(upstream, fwd2);

    SearchListenerBase::SPtr base2 = fwd2;
    SearchListenerBase::SPtr base1 = fwd1;

    EXPECT_CALL(reply, push(Matcher<unity::scopes::CategorisedResult const&>(ResultProp("title", "scope1")))).Times(3);
    EXPECT_CALL(reply, push(Matcher<unity::scopes::CategorisedResult const&>(ResultProp("title", "scope2")))).Times(10);

    // simulate a bunch of results coming from scope2
    {

        Category::SCPtr cat = std::make_shared<unity::scopes::testing::Category>("scope2catA", "Scope2", "", renderer);
        base2->push(cat);


        for (int i = 0; i<10; i++)
        {
            CategorisedResult res(cat);
            res.set_uri("http://foobar.com/" + std::to_string(i));
            res.set_title("scope2");
            base2->push(res);
        }
    }

    // now a few results coming from scope1
    {

        Category::SCPtr cat = std::make_shared<unity::scopes::testing::Category>("scope1catB", "Scope1", "", renderer);
        base1->push(cat);

        for (int i = 0; i<3; i++)
        {
            CategorisedResult res(cat);
            res.set_uri("scope1");
            res.set_title("scope1");

            base1->push(res);
        }
    }

    CompletionDetails status(CompletionDetails::CompletionStatus::OK);
    base1->finished(status);
    base2->finished(status);
}


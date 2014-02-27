/*
 * Copyright (C) 2013 Canonical Ltd
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

#include <unity/scopes/internal/RuntimeImpl.h>
#include <unity/scopes/internal/ScopeImpl.h>
#include <unity/scopes/FilterOption.h>
#include <unity/scopes/SearchMetadata.h>
#include <unity/UnityExceptions.h>
#include <gtest/gtest.h>
#include <TestScope.h>

using namespace unity::scopes;
using namespace unity::scopes::internal;

class WaitUntilFinished
{
public:
    void wait_until_finished()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        cond_.wait(lock, [this] { return this->query_complete_; });
    }

protected:
    void notify()
    {
        // Signal that the query has completed.
        std::unique_lock<std::mutex> lock(mutex_);
        query_complete_ = true;
        cond_.notify_one();
    }

private:
    bool query_complete_;
    std::mutex mutex_;
    std::condition_variable cond_;
};

class SearchReceiver : public SearchListenerBase, public WaitUntilFinished
{
public:
    virtual void push(CategorisedResult /* result */) override {}

    virtual void push(Filters const& filters, FilterState const& filter_state) override
    {
        this->filters = filters;
        this->filter_state = filter_state;
    }

    virtual void finished(ListenerBase::Reason /* reason */, std::string const& /* error_message */) override
    {
        notify();
    }

    Filters filters;
    FilterState filter_state;
};

void scope_thread(RuntimeImpl::SPtr const& rt)
{
    TestScope scope;
    rt->run_scope(&scope);
}

TEST(Filters, scope)
{
    // parent: connect to scope and run a query
    auto rt = internal::RuntimeImpl::create("", "Runtime.ini");
    auto mw = rt->factory()->create("TestScope", "Zmq", "Zmq.ini");
    mw->start();
    auto proxy = mw->create_scope_proxy("TestScope");
    auto scope = internal::ScopeImpl::create(proxy, rt.get(), "TestScope");

    SearchMetadata hints("pl", "phone");
    auto receiver = std::make_shared<SearchReceiver>();
    auto ctrl = scope->search("test", hints, receiver);
    receiver->wait_until_finished();

    auto filter_state = receiver->filter_state; // copy filter state, it will be sent with 2nd query
    {
        auto filters = receiver->filters;
        EXPECT_EQ(1u, filters.size());
        EXPECT_EQ("f1", filters.front()->id());
        auto filter_type = filters.front()->filter_type();
        EXPECT_EQ("option_selector", filter_type);
        auto selector = std::dynamic_pointer_cast<const OptionSelectorFilter>(filters.front());
        EXPECT_EQ(2u, selector->options().size());
        EXPECT_EQ(0u, selector->active_options(filter_state).size());

        auto option1 = selector->options().front();
        selector->update_state(filter_state, option1, true); // activate filter option
    }

    // send 2nd query, this time with filter state (one active option)
    receiver = std::make_shared<SearchReceiver>();
    ctrl = scope->search("test2", filter_state, hints, receiver);
    receiver->wait_until_finished();
    {
        auto filters = receiver->filters;
        auto filter_state2 = receiver->filter_state;
        auto selector = std::dynamic_pointer_cast<const OptionSelectorFilter>(filters.front());
        EXPECT_EQ(1u, selector->active_options(filter_state2).size());
        auto option1 = *(selector->active_options(filter_state2).begin());
        EXPECT_EQ("o1", option1->id());
    }
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    RuntimeImpl::SPtr rt = move(RuntimeImpl::create("TestScope", "Runtime.ini"));
    std::thread scope_t(scope_thread, rt);
    auto rc = RUN_ALL_TESTS();
    rt->destroy();
    scope_t.join();
    return rc;
}

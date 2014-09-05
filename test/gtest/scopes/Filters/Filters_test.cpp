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

#include <unity/scopes/internal/RegistryObject.h>
#include <unity/scopes/internal/RuntimeImpl.h>
#include <unity/scopes/internal/ScopeImpl.h>
#include <unity/scopes/internal/FilterBaseImpl.h>
#include <unity/scopes/FilterOption.h>
#include <unity/scopes/RadioButtonsFilter.h>
#include <unity/scopes/RatingFilter.h>
#include <unity/scopes/SwitchFilter.h>
#include <unity/scopes/ValueSliderFilter.h>
#include <unity/scopes/SearchMetadata.h>
#include <unity/UnityExceptions.h>
#include <gtest/gtest.h>
#include <TestScope.h>

using namespace unity::scopes;
using namespace unity::scopes::experimental;
using namespace unity::scopes::internal;

class WaitUntilFinished
{
public:
    void wait_until_finished()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        auto ok = cond_.wait_for(lock, std::chrono::seconds(2), [this] { return this->query_complete_; });
        ASSERT_TRUE(ok) << "query did not complete after 2 seconds";
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
    bool query_complete_ = false;
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

    virtual void finished(CompletionDetails const& details) override
    {
        ASSERT_EQ(CompletionDetails::OK, details.status()) << details.message();
        notify();
    }

    Filters filters;
    FilterState filter_state;
};

template <typename ScopeType>
struct RaiiScopeThread
{
    ScopeType scope;
    Runtime::UPtr runtime;
    std::thread scope_thread;

    RaiiScopeThread(Runtime::UPtr rt, std::string const& configfile)
        : runtime(move(rt)),
          scope_thread([this, configfile]{ runtime->run_scope(&scope, configfile, ""); })
    {
    }

    ~RaiiScopeThread()
    {
        runtime->destroy();
        scope_thread.join();
    }
};

std::shared_ptr<core::posix::SignalTrap> trap(core::posix::trap_signals_for_all_subsequent_threads({core::posix::Signal::sig_chld}));
std::unique_ptr<core::posix::ChildProcess::DeathObserver> death_observer(core::posix::ChildProcess::DeathObserver::create_once_with_signal_trap(trap));

RuntimeImpl::SPtr run_test_registry()
{
    RuntimeImpl::SPtr runtime = RuntimeImpl::create("TestRegistry", TEST_DIR "/Runtime.ini");
    MiddlewareBase::SPtr middleware = runtime->factory()->create("TestRegistry", "Zmq", TEST_DIR "/Zmq.ini");
    RegistryObject::SPtr reg_obj(std::make_shared<RegistryObject>(*death_observer, std::make_shared<Executor>(), middleware));
    middleware->add_registry_object("TestRegistry", reg_obj);
    return runtime;
}

TEST(Filters, scope)
{
    auto reg_rt = run_test_registry();

    auto scope_rt = Runtime::create_scope_runtime("TestScope", TEST_DIR "/Runtime.ini");
    RaiiScopeThread<TestScope> scope_thread(move(scope_rt), TEST_DIR "/Runtime.ini");

    // parent: connect to scope and run a query
    auto rt = internal::RuntimeImpl::create("", TEST_DIR "/Runtime.ini");
    auto mw = rt->factory()->create("TestScope", "Zmq", TEST_DIR "/Zmq.ini");
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
        ASSERT_EQ(1u, filters.size());
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
        ASSERT_EQ(1u, selector->active_options(filter_state2).size());
        auto option1 = *(selector->active_options(filter_state2).begin());
        EXPECT_EQ("o1", option1->id());
    }
}

TEST(Filters, deserialize)
{
    // check that FilterBaseImpl::deserialize() creates valid instances of all filter types
    {
        OptionSelectorFilter::SPtr filter1 = OptionSelectorFilter::create("f1", "Options", false);
        auto option1 = filter1->add_option("1", "Option 1");
        auto var = filter1->serialize();

        auto f = internal::FilterBaseImpl::deserialize(var);
        EXPECT_TRUE(std::dynamic_pointer_cast<OptionSelectorFilter const>(f) != nullptr);
        EXPECT_EQ("option_selector", f->filter_type());

        const Filters filters {filter1};
        EXPECT_NO_THROW(internal::FilterBaseImpl::validate_filters(filters));
    }

    {
        RadioButtonsFilter::SPtr filter1 = RadioButtonsFilter::create("f1", "Options");
        auto option1 = filter1->add_option("1", "Option 1");
        auto var = filter1->serialize();

        auto f = internal::FilterBaseImpl::deserialize(var);
        EXPECT_TRUE(std::dynamic_pointer_cast<RadioButtonsFilter const>(f) != nullptr);
        EXPECT_EQ("radio_buttons", f->filter_type());

        const Filters filters {filter1};
        EXPECT_NO_THROW(internal::FilterBaseImpl::validate_filters(filters));
    }

    {
        RatingFilter::SPtr filter1 = RatingFilter::create("f1", "Options", 5);
        auto var = filter1->serialize();

        auto f = internal::FilterBaseImpl::deserialize(var);
        EXPECT_TRUE(std::dynamic_pointer_cast<RatingFilter const>(f) != nullptr);
        EXPECT_EQ("rating", f->filter_type());

        const Filters filters {filter1};
        EXPECT_NO_THROW(internal::FilterBaseImpl::validate_filters(filters));
    }

    {
        SwitchFilter::SPtr filter1 = SwitchFilter::create("f1", "Latest");
        auto var = filter1->serialize();

        auto f = internal::FilterBaseImpl::deserialize(var);
        EXPECT_TRUE(std::dynamic_pointer_cast<SwitchFilter const>(f) != nullptr);
        EXPECT_EQ("switch", f->filter_type());

        const Filters filters {filter1};
        EXPECT_NO_THROW(internal::FilterBaseImpl::validate_filters(filters));
    }

    {
        ValueSliderFilter::SPtr filter1 = ValueSliderFilter::create("f1", "Max size", "Less than %f", 0.0f, 100.0f);
        auto var = filter1->serialize();

        auto f = internal::FilterBaseImpl::deserialize(var);
        EXPECT_TRUE(std::dynamic_pointer_cast<ValueSliderFilter const>(f) != nullptr);
        EXPECT_EQ("value_slider", f->filter_type());

        const Filters filters {filter1};
        EXPECT_NO_THROW(internal::FilterBaseImpl::validate_filters(filters));
    }

    {
        // invalid data (no filter_type)
        VariantMap var;
        EXPECT_THROW(internal::FilterBaseImpl::deserialize(var), unity::LogicException);
    }
}

TEST(Filters, validate)
{
    {
        FilterBase::SPtr filter1 = OptionSelectorFilter::create("f1", "Options", false);
        const Filters filters {filter1};
        EXPECT_THROW(internal::FilterBaseImpl::validate_filters(filters), unity::LogicException);
    }

    {
        FilterBase::SPtr filter1 = RadioButtonsFilter::create("f1", "Options");
        const Filters filters {filter1};
        EXPECT_THROW(internal::FilterBaseImpl::validate_filters(filters), unity::LogicException);
    }

    {
        FilterBase::SPtr filter1 = RatingFilter::create("f1", "Options");
        const Filters filters {filter1};
        EXPECT_THROW(internal::FilterBaseImpl::validate_filters(filters), unity::LogicException);
    }
}

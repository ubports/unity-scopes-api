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
#include <unity/scopes/internal/ValueSliderLabelsImpl.h>
#include <unity/scopes/FilterOption.h>
#include <unity/scopes/RadioButtonsFilter.h>
#include <unity/scopes/RangeInputFilter.h>
#include <unity/scopes/RatingFilter.h>
#include <unity/scopes/RangeInputFilter.h>
#include <unity/scopes/SwitchFilter.h>
#include <unity/scopes/ValueSliderFilter.h>
#include <unity/scopes/ValueSliderLabels.h>
#include <unity/scopes/SearchMetadata.h>
#include <unity/UnityExceptions.h>
#include <TestScope.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <gtest/gtest.h>
#pragma GCC diagnostic pop

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
    virtual void push(CategorisedResult result) override
    {
        result_uri = result.uri();
    }

    virtual void push(Filters const& filters, FilterState const&) override
    {
        this->filters2 = filters;
    }

    virtual void push(Filters const& filters) override
    {
        this->filters = filters;
    }

    virtual void finished(CompletionDetails const& details) override
    {
        ASSERT_EQ(CompletionDetails::OK, details.status()) << details.message();
        notify();
    }

    Filters filters;
    Filters filters2;
    std::string result_uri;
};

template <typename ScopeType>
struct RaiiScopeThread
{
    ScopeType scope;
    Runtime::UPtr runtime;
    std::thread scope_thread;

    RaiiScopeThread(Runtime::UPtr rt)
        : runtime(move(rt)),
          scope_thread([this]{ runtime->run_scope(&scope, ""); })
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
    RaiiScopeThread<TestScope> scope_thread(move(scope_rt));

    // parent: connect to scope and run a query
    auto rt = internal::RuntimeImpl::create("", TEST_DIR "/Runtime.ini");
    auto mw = rt->factory()->create("TestScope", "Zmq", TEST_DIR "/Zmq.ini");
    mw->start();
    auto proxy = mw->create_scope_proxy("TestScope");
    auto scope = internal::ScopeImpl::create(proxy, "TestScope");

    SearchMetadata hints("pl", "phone");
    auto receiver = std::make_shared<SearchReceiver>();
    auto ctrl = scope->search("test", hints, receiver);
    receiver->wait_until_finished();

    EXPECT_EQ("no options active", receiver->result_uri);

    FilterState filter_state;
    {
        // make sure the deprecated API push method for filters still works
        ASSERT_EQ(1u, receiver->filters2.size());

        auto filters = receiver->filters;
        ASSERT_EQ(1u, filters.size());
        EXPECT_EQ("f1", filters.front()->id());
        auto filter_type = filters.front()->filter_type();
        EXPECT_EQ("option_selector", filter_type);
        auto selector = std::dynamic_pointer_cast<const OptionSelectorFilter>(filters.front());
        EXPECT_EQ(2u, selector->options().size());
        EXPECT_EQ(0u, selector->active_options(filter_state).size());
        EXPECT_TRUE(selector->filter_group() != nullptr);
        EXPECT_EQ("g1", selector->filter_group()->id());
        EXPECT_EQ("Group", selector->filter_group()->label());

        auto option1 = selector->options().front();
        selector->update_state(filter_state, option1, true); // activate filter option
    }

    // send 2nd query, this time with filter state (one active option)
    receiver = std::make_shared<SearchReceiver>();
    ctrl = scope->search("test2", filter_state, hints, receiver);
    receiver->wait_until_finished();
    {
        auto filters = receiver->filters;
        ASSERT_EQ(1u, filters.size());
        auto selector = std::dynamic_pointer_cast<const OptionSelectorFilter>(filters.front());
        EXPECT_EQ("option o1 active", receiver->result_uri);
    }
}

TEST(Filters, deserialize)
{
    // check that FilterBase::deserialize() creates valid instances of all filter types
    {
        OptionSelectorFilter::SPtr filter1 = OptionSelectorFilter::create("f1", "Options", false);
        auto option1 = filter1->add_option("1", "Option 1");
        auto var = filter1->serialize();

        auto f = FilterBase::deserialize(var);
        EXPECT_TRUE(std::dynamic_pointer_cast<OptionSelectorFilter const>(f) != nullptr);
        EXPECT_EQ("option_selector", f->filter_type());

        const Filters filters {filter1};
        EXPECT_NO_THROW(internal::FilterBaseImpl::validate_filters(filters));
    }

    {
        RangeInputFilter::SPtr filter1 = RangeInputFilter::create("f1", Variant(1.0f), Variant(20.0f), "A", "B", "C", "D", "E");
        auto var = filter1->serialize();

        auto f = FilterBase::deserialize(var);
        EXPECT_TRUE(std::dynamic_pointer_cast<RangeInputFilter const>(f) != nullptr);
        EXPECT_EQ("range_input", f->filter_type());

        const Filters filters {filter1};
        EXPECT_NO_THROW(internal::FilterBaseImpl::validate_filters(filters));
    }

    {
        RadioButtonsFilter::SPtr filter1 = RadioButtonsFilter::create("f1", "Options");
        auto option1 = filter1->add_option("1", "Option 1");
        auto var = filter1->serialize();

        auto f = FilterBase::deserialize(var);
        EXPECT_TRUE(std::dynamic_pointer_cast<RadioButtonsFilter const>(f) != nullptr);
        EXPECT_EQ("radio_buttons", f->filter_type());

        const Filters filters {filter1};
        EXPECT_NO_THROW(internal::FilterBaseImpl::validate_filters(filters));
    }

    {
        RatingFilter::SPtr filter1 = RatingFilter::create("f1", "Options", 5);
        auto var = filter1->serialize();

        auto f = FilterBase::deserialize(var);
        EXPECT_TRUE(std::dynamic_pointer_cast<RatingFilter const>(f) != nullptr);
        EXPECT_EQ("rating", f->filter_type());

        const Filters filters {filter1};
        EXPECT_NO_THROW(internal::FilterBaseImpl::validate_filters(filters));
    }

    {
        SwitchFilter::SPtr filter1 = SwitchFilter::create("f1", "Latest");
        auto var = filter1->serialize();

        auto f = FilterBase::deserialize(var);
        EXPECT_TRUE(std::dynamic_pointer_cast<SwitchFilter const>(f) != nullptr);
        EXPECT_EQ("switch", f->filter_type());

        const Filters filters {filter1};
        EXPECT_NO_THROW(internal::FilterBaseImpl::validate_filters(filters));
    }

    {
        ValueSliderFilter::SPtr filter1 = ValueSliderFilter::create("f1", 0, 100, 100, ValueSliderLabels("Min", "Max"));
        auto var = filter1->serialize();

        auto f = FilterBase::deserialize(var);
        EXPECT_TRUE(std::dynamic_pointer_cast<ValueSliderFilter const>(f) != nullptr);
        EXPECT_EQ("value_slider", f->filter_type());

        const Filters filters {filter1};
        EXPECT_NO_THROW(internal::FilterBaseImpl::validate_filters(filters));
    }

    {
        // invalid data (no filter_type)
        VariantMap var;
        EXPECT_THROW(FilterBase::deserialize(var), unity::LogicException);
    }
}

TEST(Filters, filter_group)
{
    {
        auto grp1 = FilterGroup::create("g1", "Group 1");
        auto grp2 = FilterGroup::create("g2", "Group 2");
        FilterBase::SPtr filter1 = OptionSelectorFilter::create("f1", "Options", grp1, false);
        FilterBase::SPtr filter2 = RangeInputFilter::create("f2", "a", "b", "c", "d", "e", grp1);
        FilterBase::SPtr filter3 = ValueSliderFilter::create("f3", 1, 100, 100, ValueSliderLabels("Min", "Max"), grp1);
        FilterBase::SPtr filter4 = RangeInputFilter::create("f4", Variant(1.0f), Variant(10.0f), "a", "b", "c", "d", "e", grp2);

        EXPECT_TRUE(filter1->filter_group() != nullptr);
        EXPECT_TRUE(filter2->filter_group() != nullptr);
        EXPECT_TRUE(filter3->filter_group() != nullptr);
        EXPECT_EQ(filter1->filter_group(), filter2->filter_group());
        EXPECT_EQ(filter1->filter_group(), filter3->filter_group());
        EXPECT_EQ("g1", filter1->filter_group()->id());
        EXPECT_EQ("Group 1", filter1->filter_group()->label());

        EXPECT_TRUE(filter4->filter_group() != nullptr);
        EXPECT_EQ("g2", filter4->filter_group()->id());
        EXPECT_EQ("Group 2", filter4->filter_group()->label());

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

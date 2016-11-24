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

#include <unity/scopes/CategorisedResult.h>
#include <unity/scopes/Category.h>
#include <unity/scopes/CategoryRenderer.h>
#include <unity/scopes/internal/CategoryRegistry.h>
#include <unity/scopes/internal/CategorisedResultImpl.h>
#include <unity/scopes/internal/RegistryObject.h>
#include <unity/scopes/internal/ResultReplyObject.h>
#include <unity/scopes/internal/RuntimeImpl.h>
#include <unity/scopes/internal/ScopeImpl.h>
#include <unity/scopes/ActionMetadata.h>
#include <unity/UnityExceptions.h>
#include <functional>
#include <unity/scopes/testing/Result.h>
#include <TestScope.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <gtest/gtest.h>
#pragma GCC diagnostic pop

using namespace unity::scopes;
using namespace unity::scopes::internal;

class DummyReceiver : public SearchListenerBase
{
public:
    DummyReceiver(std::function<void(CategorisedResult)> push_func)
    {
        push_func_ = push_func;
    }

    void push(CategorisedResult result) override
    {
        push_func_(result);
    }

    void finished(CompletionDetails const&) override {}

    std::function<void(CategorisedResult)> push_func_;
};

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
        this->result = std::make_shared<Result>(result);
    }

    virtual void finished(CompletionDetails const& details) override
    {
        EXPECT_EQ(CompletionDetails::OK, details.status()) << details.message();
        notify();
    }

    std::shared_ptr<Result> result;
};

class ActivationReceiver : public ActivationListenerBase, public WaitUntilFinished
{
public:
    virtual void activated(ActivationResponse const& response) override
    {
        this->response = std::make_shared<ActivationResponse>(response);
    }

    void finished(CompletionDetails const&) override
    {
        notify();
    }

    std::shared_ptr<ActivationResponse> response;
};

// helper function to serialize category and result into a dict expected by ResultReplyObject
VariantMap serialize_data(Category::SCPtr category, CategorisedResult const& result)
{
    VariantMap var;
    var["category"] = category->serialize();
    var["result"] = result.serialize();
    return var;
}

TEST(Activation, exceptions)
{
    CategoryRegistry reg;
    CategoryRenderer rdr;
    auto cat = reg.register_category("1", "title", "icon", nullptr, rdr);
    {
        CategorisedResult result(cat);
        result.set_uri("http://ubuntu.com");
        result.set_dnd_uri("http://canonical.com");

        EXPECT_TRUE(result.direct_activation());
        result.set_intercept_activation();

        EXPECT_FALSE(result.direct_activation());
        EXPECT_THROW(result.target_scope_proxy(), unity::LogicException);
    }
}

TEST(Activation, direct_activation)
{
    CategoryRegistry reg;
    CategoryRenderer rdr;
    auto cat = reg.register_category("1", "title", "icon", nullptr, rdr);

    // activation interception not set
    {
        std::shared_ptr<CategorisedResult> received_result;
        auto df = []() -> void {};
        auto runtime = internal::RuntimeImpl::create("", TEST_DIR "/Runtime.ini");
        auto receiver = std::make_shared<DummyReceiver>([&received_result](CategorisedResult result)
                {
                    received_result.reset(new CategorisedResult(result));
                });
        internal::ResultReplyObject reply(receiver, runtime.get(), "ipc:///tmp/scope-foo#scope-foo!c=Scope", 0);
        reply.set_disconnect_function(df);

        {
            CategorisedResult result(cat);
            result.set_uri("http://ubuntu.com");
            result.set_dnd_uri("http://canonical.com");

            // push category and result through ResultReplyObject
            reply.process_data(serialize_data(cat, result));
        }

        EXPECT_TRUE(received_result != nullptr);
        EXPECT_TRUE(received_result->direct_activation());
        EXPECT_TRUE(received_result->target_scope_proxy()->to_string().find("scope-foo") != std::string::npos); // direct activation, but name is still available
    }
}

// direct activation with an aggregator; aggregator scope doesn't store the original result, just passes it
TEST(Activation, direct_activation_agg_scope_doesnt_store)
{
    CategoryRegistry reg;
    CategoryRenderer rdr;
    auto cat = reg.register_category("1", "title", "icon", nullptr, rdr);

    {
        std::shared_ptr<CategorisedResult> received_result;
        auto df = []() -> void {};
        auto runtime = internal::RuntimeImpl::create("", TEST_DIR "/Runtime.ini");
        auto receiver = std::make_shared<DummyReceiver>([&received_result](CategorisedResult result)
                {
                    received_result.reset(new CategorisedResult(result));
                });
        internal::ResultReplyObject reply(receiver, runtime.get(), "ipc:///tmp/scope-foo#scope-foo!c=Scope", 0);
        reply.set_disconnect_function(df);

        {
            CategorisedResult result(cat);
            result.set_uri("http://ubuntu.com");
            result.set_dnd_uri("http://canonical.com");

            // push category and result through ResultReplyObject
            reply.process_data(serialize_data(cat, result));
        }

        EXPECT_TRUE(received_result != nullptr);
        EXPECT_TRUE(received_result->direct_activation());
        EXPECT_TRUE(received_result->target_scope_proxy()->to_string().find("ipc:///tmp/scope-foo#scope-foo!c=Scope") != std::string::npos);

        // simulate aggregator scope
        std::shared_ptr<CategorisedResult> agg_received_result;
        auto aggreceiver = std::make_shared<DummyReceiver>([&agg_received_result](CategorisedResult result)
                {
                    agg_received_result.reset(new CategorisedResult(result));
                });
        internal::ResultReplyObject aggreply(aggreceiver, runtime.get(), "ipc:///tmp/scope-bar#scope-bar!c=Scope", 0);
        aggreply.set_disconnect_function(df);

        {
            // push category and unchanged result through ResultReplyObject
            aggreply.process_data(serialize_data(cat, *received_result));
        }

        EXPECT_TRUE(agg_received_result != nullptr);
        EXPECT_FALSE(agg_received_result->has_stored_result());
        EXPECT_TRUE(agg_received_result->direct_activation());
        EXPECT_TRUE(agg_received_result->target_scope_proxy()->to_string().find("scope-foo") != std::string::npos);
    }
}

// direct activation with an aggregator; aggregator scope stores the original result
TEST(Activation, direct_activation_agg_scope_stores)
{
    CategoryRegistry reg;
    CategoryRenderer rdr;
    auto cat = reg.register_category("1", "title", "icon", nullptr, rdr);

    {
        std::shared_ptr<CategorisedResult> received_result;
        auto df = []() -> void {};
        auto runtime = internal::RuntimeImpl::create("", TEST_DIR "/Runtime.ini");
        auto receiver = std::make_shared<DummyReceiver>([&received_result](CategorisedResult result)
                {
                    received_result.reset(new CategorisedResult(result));
                });
        internal::ResultReplyObject reply(receiver, runtime.get(), "ipc:///scope-foo#scope-foo!c=Scope", 0);
        reply.set_disconnect_function(df);

        {
            CategorisedResult result(cat);
            result.set_uri("http://ubuntu.com");
            result.set_dnd_uri("http://canonical.com");

            // push category and result through ResultReplyObject
            reply.process_data(serialize_data(cat, result));
        }

        EXPECT_TRUE(received_result != nullptr);
        EXPECT_TRUE(received_result->direct_activation());
        EXPECT_TRUE(received_result->target_scope_proxy()->to_string().find("scope-foo") != std::string::npos);

        // simulate aggregator scope
        std::shared_ptr<CategorisedResult> agg_received_result;
        auto aggreceiver = std::make_shared<DummyReceiver>([&agg_received_result](CategorisedResult result)
                {
                    agg_received_result.reset(new CategorisedResult(result));
                });
        internal::ResultReplyObject aggreply(aggreceiver, runtime.get(), "ipc:///scope-bar#scope-bar!c=Scope", 0);
        aggreply.set_disconnect_function(df);

        {
            CategorisedResult outerresult(cat);
            outerresult.set_uri("http://ubuntu.com/2");
            outerresult.set_dnd_uri("http://canonical.com/2");
            outerresult.store(*received_result, false);

            // push category and result through ResultReplyObject
            aggreply.process_data(serialize_data(cat, outerresult));
        }

        EXPECT_TRUE(agg_received_result != nullptr);
        EXPECT_TRUE(agg_received_result->has_stored_result());
        EXPECT_TRUE(agg_received_result->direct_activation());
        // target_scope_proxy points to the leaf scope
        EXPECT_TRUE(agg_received_result->target_scope_proxy()->to_string().find("scope-foo") != std::string::npos);
    }
}

// an aggregator scope just passes the result and doesn't set InterceptActivation
TEST(Activation, agg_scope_doesnt_store_and_doesnt_intercept)
{
    CategoryRegistry reg;
    CategoryRenderer rdr;
    auto cat = reg.register_category("1", "title", "icon", nullptr, rdr);

    {
        std::shared_ptr<CategorisedResult> received_result;
        auto df = []() -> void {};
        auto runtime = internal::RuntimeImpl::create("", TEST_DIR "/Runtime.ini");
        auto receiver = std::make_shared<DummyReceiver>([&received_result](CategorisedResult result)
                {
                    received_result.reset(new CategorisedResult(result));
                });
        internal::ResultReplyObject reply(receiver, runtime.get(), "ipc:///tmp/scope-foo#scope-foo!c=Scope", 0);
        reply.set_disconnect_function(df);

        {
            CategorisedResult result(cat);
            result.set_uri("http://ubuntu.com");
            result.set_dnd_uri("http://canonical.com");
            result.set_intercept_activation();

            // push category and result through ResultReplyObject
            reply.process_data(serialize_data(cat, result));
        }

        EXPECT_TRUE(received_result != nullptr);
        EXPECT_FALSE(received_result->direct_activation());
        EXPECT_TRUE(received_result->target_scope_proxy()->to_string().find("scope-foo") != std::string::npos);

        // simulate aggregator scope
        std::shared_ptr<CategorisedResult> agg_received_result;
        auto aggreceiver = std::make_shared<DummyReceiver>([&agg_received_result](CategorisedResult result)
                {
                    agg_received_result.reset(new CategorisedResult(result));
                });
        internal::ResultReplyObject aggreply(aggreceiver, runtime.get(), "ipc:///tmp/scope-bar#scope-bar!c=Scope", 0);
        aggreply.set_disconnect_function(df);

        {
            // push category and unchanged result through ResultReplyObject
            aggreply.process_data(serialize_data(cat, *received_result));
        }

        EXPECT_TRUE(agg_received_result != nullptr);
        EXPECT_FALSE(agg_received_result->has_stored_result());
        EXPECT_FALSE(agg_received_result->direct_activation());
        // target_scope_proxy unchanged since aggregator doesn't intercept activation
        EXPECT_TRUE(agg_received_result->target_scope_proxy()->to_string().find("scope-foo") != std::string::npos);
    }
}

// an aggregator scope just passes the result and sets InterceptActivation
TEST(Activation, agg_scope_doesnt_store_and_sets_intercept)
{
    CategoryRegistry reg;
    CategoryRenderer rdr;
    auto cat = reg.register_category("1", "title", "icon", nullptr, rdr);

    {
        std::shared_ptr<CategorisedResult> received_result;
        auto df = []() -> void {};
        auto runtime = internal::RuntimeImpl::create("", TEST_DIR "/Runtime.ini");
        auto receiver = std::make_shared<DummyReceiver>([&received_result](CategorisedResult result)
                {
                    received_result.reset(new CategorisedResult(result));
                });
        internal::ResultReplyObject reply(receiver, runtime.get(), "ipc:///tmp/scope-foo#scope-foo!c=Scope", 0);
        reply.set_disconnect_function(df);

        {
            CategorisedResult result(cat);
            result.set_uri("http://ubuntu.com");
            result.set_dnd_uri("http://canonical.com");
            result.set_intercept_activation();

            // push category and result through ResultReplyObject
            reply.process_data(serialize_data(cat, result));
        }

        EXPECT_TRUE(received_result != nullptr);
        EXPECT_FALSE(received_result->direct_activation());
        EXPECT_TRUE(received_result->target_scope_proxy()->to_string().find("scope-foo") != std::string::npos);

        // simulate aggregator scope
        std::shared_ptr<CategorisedResult> agg_received_result;
        auto aggreceiver = std::make_shared<DummyReceiver>([&agg_received_result](CategorisedResult result)
                {
                    agg_received_result.reset(new CategorisedResult(result));
                });
        internal::ResultReplyObject aggreply(aggreceiver, runtime.get(), "ipc:///tmp/scope-bar#scope-bar!c=Scope", 0);
        aggreply.set_disconnect_function(df);

        {
            received_result->set_intercept_activation(); // agg scope want to receive activation
            // push category and unchanged result through ResultReplyObject
            aggreply.process_data(serialize_data(cat, *received_result));
        }

        EXPECT_TRUE(agg_received_result != nullptr);
        EXPECT_FALSE(agg_received_result->has_stored_result());
        EXPECT_FALSE(agg_received_result->direct_activation());
        // target_scope_proxy changed since aggregator intercepts activation
        EXPECT_TRUE(agg_received_result->target_scope_proxy()->to_string().find("scope-bar") != std::string::npos);
    }
}

// an aggregator scope stores the original result but doesn't set InterceptActivation
TEST(Activation, agg_scope_stores_and_doesnt_intercept)
{
    CategoryRegistry reg;
    CategoryRenderer rdr;
    auto cat = reg.register_category("1", "title", "icon", nullptr, rdr);

    {
        std::shared_ptr<CategorisedResult> received_result;
        auto df = []() -> void {};
        auto runtime = internal::RuntimeImpl::create("", TEST_DIR "/Runtime.ini");
        auto receiver = std::make_shared<DummyReceiver>([&received_result](CategorisedResult result)
                {
                    received_result.reset(new CategorisedResult(result));
                });
        internal::ResultReplyObject reply(receiver, runtime.get(), "ipc:///tmp/scope-foo#scope-foo!c=Scope", 0);
        reply.set_disconnect_function(df);

        {
            CategorisedResult result(cat);
            result.set_uri("http://ubuntu.com");
            result.set_dnd_uri("http://canonical.com");
            result.set_intercept_activation();

            // push category and result through ResultReplyObject
            reply.process_data(serialize_data(cat, result));
        }

        EXPECT_TRUE(received_result != nullptr);
        EXPECT_FALSE(received_result->direct_activation());
        EXPECT_TRUE(received_result->target_scope_proxy()->to_string().find("scope-foo") != std::string::npos);

        // simulate aggregator scope
        std::shared_ptr<CategorisedResult> agg_received_result;
        auto aggreceiver = std::make_shared<DummyReceiver>([&agg_received_result](CategorisedResult result)
                {
                    agg_received_result.reset(new CategorisedResult(result));
                });
        internal::ResultReplyObject aggreply(aggreceiver, runtime.get(), "ipc:///tmp/scope-bar#scope-bar!c=Scope", 0);
        aggreply.set_disconnect_function(df);

        {
            CategorisedResult outerresult(cat);
            outerresult.set_uri("http://ubuntu.com/2");
            outerresult.set_dnd_uri("http://canonical.com/2");
            outerresult.store(*received_result, false);

            // push category and result through ResultReplyObject
            aggreply.process_data(serialize_data(cat, outerresult));
        }

        EXPECT_TRUE(agg_received_result != nullptr);
        EXPECT_TRUE(agg_received_result->has_stored_result());
        EXPECT_FALSE(agg_received_result->direct_activation());
        // target_scope_proxy unchanged since aggregator doesn't intercept activation
        EXPECT_TRUE(agg_received_result->target_scope_proxy()->to_string().find("scope-foo") != std::string::npos);
    }
}

// an aggregator scope stores the original result and sets InterceptActivation
TEST(Activation, agg_scope_stores_and_intercepts)
{
    CategoryRegistry reg;
    CategoryRenderer rdr;
    auto cat = reg.register_category("1", "title", "icon", nullptr, rdr);

    {
        std::shared_ptr<CategorisedResult> received_result;
        auto df = []() -> void {};
        auto runtime = internal::RuntimeImpl::create("", TEST_DIR "/Runtime.ini");
        auto receiver = std::make_shared<DummyReceiver>([&received_result](CategorisedResult result)
                {
                    received_result.reset(new CategorisedResult(result));
                });
        internal::ResultReplyObject reply(receiver, runtime.get(), "ipc:///tmp/scope-foo#scope-foo!c=Scope", 0);
        reply.set_disconnect_function(df);

        {
            CategorisedResult result(cat);
            result.set_uri("http://ubuntu.com");
            result.set_dnd_uri("http://canonical.com");
            result.set_intercept_activation();

            // push category and result through ResultReplyObject
            reply.process_data(serialize_data(cat, result));
        }

        EXPECT_TRUE(received_result != nullptr);
        EXPECT_FALSE(received_result->direct_activation());
        EXPECT_TRUE(received_result->target_scope_proxy()->to_string().find("scope-foo") != std::string::npos);

        // simulate aggregator scope
        std::shared_ptr<CategorisedResult> agg_received_result;
        auto aggreceiver = std::make_shared<DummyReceiver>([&agg_received_result](CategorisedResult result)
                {
                    agg_received_result.reset(new CategorisedResult(result));
                });
        internal::ResultReplyObject aggreply(aggreceiver, runtime.get(), "ipc:///tmp/scope-bar#scope-bar!c=Scope", 0);
        aggreply.set_disconnect_function(df);

        {
            CategorisedResult outerresult(cat);
            outerresult.set_uri("http://ubuntu.com/2");
            outerresult.set_dnd_uri("http://canonical.com/2");
            outerresult.store(*received_result, false);
            outerresult.set_intercept_activation();

            // push category and result through ResultReplyObject
            aggreply.process_data(serialize_data(cat, outerresult));
        }

        EXPECT_TRUE(agg_received_result != nullptr);
        EXPECT_TRUE(agg_received_result->has_stored_result());
        EXPECT_FALSE(agg_received_result->direct_activation());
        // scope changed to agg scope because it intercepts activation
        EXPECT_TRUE(agg_received_result->target_scope_proxy()->to_string().find("scope-bar") != std::string::npos);
    }
}

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

// does actual activation with a test scope
TEST(Activation, scope)
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

    VariantMap hints;
    auto receiver = std::make_shared<SearchReceiver>();
    auto ctrl = scope->search("test", SearchMetadata("pl", "phone"), receiver);
    receiver->wait_until_finished();

    auto result = receiver->result;
    ASSERT_TRUE(result != nullptr);
    EXPECT_FALSE(result->direct_activation());
    EXPECT_EQ("uri", result->uri());
    EXPECT_EQ("dnd_uri", result->dnd_uri());

    auto target = result->target_scope_proxy();
    EXPECT_TRUE(target != nullptr);
    EXPECT_TRUE(target->to_string().find("TestScope") != std::string::npos);

    // activate result
    {
        auto act_receiver = std::make_shared<ActivationReceiver>();
        hints["iron"] = "maiden";

        ActionMetadata metadata("C", "phone");
        metadata.set_scope_data(Variant(hints));
        ctrl = target->activate(*result, metadata, act_receiver);
        act_receiver->wait_until_finished();

        auto response = act_receiver->response;
        EXPECT_TRUE(response != nullptr);
        EXPECT_EQ(ActivationResponse::Status::ShowDash, response->status());
        EXPECT_EQ("bar", response->scope_data().get_dict()["foo"].get_string());
        EXPECT_EQ("maiden", response->scope_data().get_dict()["received_hints"].get_dict()["iron"].get_string());
        EXPECT_EQ("uri", response->scope_data().get_dict()["activated_uri"].get_string());
    }

    // activate action
    {
        auto act_receiver = std::make_shared<ActivationReceiver>();
        ActionMetadata meta("en", "phone");
        hints["iron"] = "maiden";
        meta.set_scope_data(Variant(hints));
        ctrl = target->perform_action(*result, meta, "widget1", "action1", act_receiver);
        act_receiver->wait_until_finished();

        auto response = act_receiver->response;
        EXPECT_TRUE(response != nullptr);
        EXPECT_EQ(ActivationResponse::Status::ShowDash, response->status());
        EXPECT_EQ("widget1action1", response->scope_data().get_dict()["activated action"].get_string());
        EXPECT_EQ("maiden", response->scope_data().get_dict()["received_hints"].get_dict()["iron"].get_string());
        EXPECT_EQ("uri", response->scope_data().get_dict()["activated_uri"].get_string());
   }
}

TEST(Activation, result_action)
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

    VariantMap hints;
    auto receiver = std::make_shared<SearchReceiver>();
    auto ctrl = scope->search("test", SearchMetadata("pl", "phone"), receiver);
    receiver->wait_until_finished();

    auto result = receiver->result;
    ASSERT_TRUE(result != nullptr);

    auto target = result->target_scope_proxy();
    EXPECT_TRUE(target != nullptr);

    // activate result action
    {
        auto act_receiver = std::make_shared<ActivationReceiver>();
        ActionMetadata meta("en", "phone");
        hints["iron"] = "maiden";
        meta.set_scope_data(Variant(hints));
        ctrl = target->activate_result_action(*result, meta, "action1", act_receiver);
        act_receiver->wait_until_finished();

        auto response = act_receiver->response;
        EXPECT_TRUE(response != nullptr);
        EXPECT_EQ(ActivationResponse::Status::UpdateResult, response->status());
        EXPECT_EQ("uri", response->updated_result().uri());
        ASSERT_NO_THROW(response->updated_result().target_scope_proxy());
        EXPECT_EQ("action1", response->scope_data().get_dict()["activated result action"].get_string());
        EXPECT_EQ("maiden", response->scope_data().get_dict()["received_hints"].get_dict()["iron"].get_string());
    }
}

class MyActivation : public ActivationQueryBase
{
public:
    MyActivation(Result const& result, ActionMetadata const& metadata)
        : ActivationQueryBase(result, metadata)
    {
    }
};

// check that parameters to ActivationQueryBase ctor are accessible via getters
TEST(Activation, activation_query_base)
{
    unity::scopes::testing::Result res;
    ActionMetadata hints("pl", "phone");
    MyActivation act(res, hints);
    EXPECT_EQ("", act.result().uri());
    EXPECT_EQ("pl", act.action_metadata().locale());
}

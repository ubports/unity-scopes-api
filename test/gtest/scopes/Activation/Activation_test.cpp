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

#include <scopes/CategorisedResult.h>
#include <scopes/Category.h>
#include <scopes/CategoryRenderer.h>
#include <scopes/internal/CategoryRegistry.h>
#include <scopes/internal/CategorisedResultImpl.h>
#include <scopes/internal/ReplyObject.h>
#include <scopes/internal/RuntimeImpl.h>
#include <unity/UnityExceptions.h>
#include <functional>
#include <gtest/gtest.h>

using namespace unity::api::scopes;
using namespace unity::api::scopes::internal;

class DummyReceiver : public SearchListener
{
public:
    DummyReceiver(std::function<void(CategorisedResult)> push_func)
    {
        push_func_ = push_func;
    };
    void push(CategorisedResult result) override
    {
        push_func_(result);
    }
    void finished(Reason /* r */, std::string const& /* error_message */) override {}

    std::function<void(CategorisedResult)> push_func_;
};

// test activation paramaters / flags passed from scope to client
TEST(Activation, exceptions)
{
    CategoryRegistry reg;
    CategoryRenderer rdr;
    auto cat = reg.register_category("1", "title", "icon", rdr);

    // exception - activation_scope_name undefined unless result is actually transferred
    {
        CategorisedResult result(cat);
        result.set_uri("http://ubuntu.com");
        result.set_dnd_uri("http://canonical.com");

        EXPECT_TRUE(result.direct_activation());
        EXPECT_THROW(result.activation_scope_name(), unity::LogicException);

        result.set_intercept_activation();
        EXPECT_FALSE(result.direct_activation());
        EXPECT_THROW(result.activation_scope_name(), unity::LogicException);
    }
}

TEST(Activation, direct_activation)
{
    CategoryRegistry reg;
    CategoryRenderer rdr;
    auto cat = reg.register_category("1", "title", "icon", rdr);

    // activation interception not set
    {
        std::shared_ptr<CategorisedResult> received_result;
        auto df = []() -> void {};
        auto runtime = internal::RuntimeImpl::create("", "Runtime.ini");
        auto receiver = std::make_shared<DummyReceiver>([&received_result](CategorisedResult result)
                {
                    received_result.reset(new CategorisedResult(result));
                });
        internal::ResultReplyObject reply(receiver, runtime.get(), "scope-foo");
        reply.set_disconnect_function(df);

        {
            CategorisedResult result(cat);
            result.set_uri("http://ubuntu.com");
            result.set_dnd_uri("http://canonical.com");

            // push category and result through ResultReplyObject
            VariantMap var;
            var["category"] = cat->serialize();
            var["result"] = result.serialize();
            reply.process_data(var);
        }

        EXPECT_TRUE(received_result != nullptr);
        EXPECT_TRUE(received_result->direct_activation());
        EXPECT_THROW(received_result->activation_scope_name(), unity::LogicException);
    }
}

// an aggregator scope just passes the result and doesn't set InterceptActivation
TEST(Activation, agg_scope_doesnt_store_and_doesnt_intercept)
{
    CategoryRegistry reg;
    CategoryRenderer rdr;
    auto cat = reg.register_category("1", "title", "icon", rdr);

    {
        std::shared_ptr<CategorisedResult> received_result;
        auto df = []() -> void {};
        auto runtime = internal::RuntimeImpl::create("", "Runtime.ini");
        auto receiver = std::make_shared<DummyReceiver>([&received_result](CategorisedResult result)
                {
                    received_result.reset(new CategorisedResult(result));
                });
        internal::ResultReplyObject reply(receiver, runtime.get(), "scope-foo");
        reply.set_disconnect_function(df);

        {
            CategorisedResult result(cat);
            result.set_uri("http://ubuntu.com");
            result.set_dnd_uri("http://canonical.com");
            result.set_intercept_activation();

            // push category and result through ResultReplyObject
            VariantMap var;
            var["category"] = cat->serialize();
            var["result"] = result.serialize();
            reply.process_data(var);
        }

        EXPECT_TRUE(received_result != nullptr);
        EXPECT_FALSE(received_result->direct_activation());
        EXPECT_EQ("scope-foo", received_result->activation_scope_name());

        // simulate aggregator scope
        std::shared_ptr<CategorisedResult> agg_received_result;
        auto aggreceiver = std::make_shared<DummyReceiver>([&agg_received_result](CategorisedResult result)
                {
                    agg_received_result.reset(new CategorisedResult(result));
                });
        internal::ResultReplyObject aggreply(aggreceiver, runtime.get(), "scope-bar");
        aggreply.set_disconnect_function(df);

        {
            // push category and unchanged result through ResultReplyObject
            VariantMap var;
            var["category"] = cat->serialize();
            var["result"] = received_result->serialize();
            aggreply.process_data(var);
        }

        EXPECT_TRUE(agg_received_result != nullptr);
        EXPECT_FALSE(agg_received_result->has_stored_result());
        EXPECT_FALSE(agg_received_result->direct_activation());
        // activation_scope_name unchanged since aggregator doesn't intercept activation
        EXPECT_EQ("scope-foo", agg_received_result->activation_scope_name());
    }
}

// an aggregator scope just passes the result and sets InterceptActivation
TEST(Activation, agg_scope_doesnt_store_and_sets_intercept)
{
    CategoryRegistry reg;
    CategoryRenderer rdr;
    auto cat = reg.register_category("1", "title", "icon", rdr);

    {
        std::shared_ptr<CategorisedResult> received_result;
        auto df = []() -> void {};
        auto runtime = internal::RuntimeImpl::create("", "Runtime.ini");
        auto receiver = std::make_shared<DummyReceiver>([&received_result](CategorisedResult result)
                {
                    received_result.reset(new CategorisedResult(result));
                });
        internal::ResultReplyObject reply(receiver, runtime.get(), "scope-foo");
        reply.set_disconnect_function(df);

        {
            CategorisedResult result(cat);
            result.set_uri("http://ubuntu.com");
            result.set_dnd_uri("http://canonical.com");
            result.set_intercept_activation();

            // push category and result through ResultReplyObject
            VariantMap var;
            var["category"] = cat->serialize();
            var["result"] = result.serialize();
            reply.process_data(var);
        }

        EXPECT_TRUE(received_result != nullptr);
        EXPECT_FALSE(received_result->direct_activation());
        EXPECT_EQ("scope-foo", received_result->activation_scope_name());

        // simulate aggregator scope
        std::shared_ptr<CategorisedResult> agg_received_result;
        auto aggreceiver = std::make_shared<DummyReceiver>([&agg_received_result](CategorisedResult result)
                {
                    agg_received_result.reset(new CategorisedResult(result));
                });
        internal::ResultReplyObject aggreply(aggreceiver, runtime.get(), "scope-bar");
        aggreply.set_disconnect_function(df);

        {
            received_result->set_intercept_activation(); // agg scope want to receive activation
            // push category and unchanged result through ResultReplyObject
            VariantMap var;
            var["category"] = cat->serialize();
            var["result"] = received_result->serialize();
            aggreply.process_data(var);
        }

        EXPECT_TRUE(agg_received_result != nullptr);
        EXPECT_FALSE(agg_received_result->has_stored_result());
        EXPECT_FALSE(agg_received_result->direct_activation());
        // activation_scope_name unchanged since aggregator doesn't intercept activation
        EXPECT_EQ("scope-bar", agg_received_result->activation_scope_name());
    }
}

// an aggregator scope stores the original result but doesn't set InterceptActivation
TEST(Activation, agg_scope_stores_and_doesnt_intercept)
{
    CategoryRegistry reg;
    CategoryRenderer rdr;
    auto cat = reg.register_category("1", "title", "icon", rdr);

    {
        std::shared_ptr<CategorisedResult> received_result;
        auto df = []() -> void {};
        auto runtime = internal::RuntimeImpl::create("", "Runtime.ini");
        auto receiver = std::make_shared<DummyReceiver>([&received_result](CategorisedResult result)
                {
                    received_result.reset(new CategorisedResult(result));
                });
        internal::ResultReplyObject reply(receiver, runtime.get(), "scope-foo");
        reply.set_disconnect_function(df);

        {
            CategorisedResult result(cat);
            result.set_uri("http://ubuntu.com");
            result.set_dnd_uri("http://canonical.com");
            result.set_intercept_activation();

            // push category and result through ResultReplyObject
            VariantMap var;
            var["category"] = cat->serialize();
            var["result"] = result.serialize();
            reply.process_data(var);
        }

        EXPECT_TRUE(received_result != nullptr);
        EXPECT_FALSE(received_result->direct_activation());
        EXPECT_EQ("scope-foo", received_result->activation_scope_name());

        // simulate aggregator scope
        std::shared_ptr<CategorisedResult> agg_received_result;
        auto aggreceiver = std::make_shared<DummyReceiver>([&agg_received_result](CategorisedResult result)
                {
                    agg_received_result.reset(new CategorisedResult(result));
                });
        internal::ResultReplyObject aggreply(aggreceiver, runtime.get(), "scope-bar");
        aggreply.set_disconnect_function(df);

        {
            CategorisedResult outerresult(cat);
            outerresult.set_uri("http://ubuntu.com/2");
            outerresult.set_dnd_uri("http://canonical.com/2");
            outerresult.store(*received_result, false);

            // push category and result through ResultReplyObject
            VariantMap var;
            var["category"] = cat->serialize();
            var["result"] = outerresult.serialize();
            aggreply.process_data(var);
        }

        EXPECT_TRUE(agg_received_result != nullptr);
        EXPECT_TRUE(agg_received_result->has_stored_result());
        EXPECT_FALSE(agg_received_result->direct_activation());
        // activation_scope_name unchanged since aggregator doesn't intercept activation
        EXPECT_EQ("scope-foo", agg_received_result->activation_scope_name());
    }
}

// an aggregator scope stores the original result and sets InterceptActivation
TEST(Activation, agg_scope_stores_and_intercepts)
{
    CategoryRegistry reg;
    CategoryRenderer rdr;
    auto cat = reg.register_category("1", "title", "icon", rdr);

    {
        std::shared_ptr<CategorisedResult> received_result;
        auto df = []() -> void {};
        auto runtime = internal::RuntimeImpl::create("", "Runtime.ini");
        auto receiver = std::make_shared<DummyReceiver>([&received_result](CategorisedResult result)
                {
                    received_result.reset(new CategorisedResult(result));
                });
        internal::ResultReplyObject reply(receiver, runtime.get(), "scope-foo");
        reply.set_disconnect_function(df);

        {
            CategorisedResult result(cat);
            result.set_uri("http://ubuntu.com");
            result.set_dnd_uri("http://canonical.com");
            result.set_intercept_activation();

            // push category and result through ResultReplyObject
            VariantMap var;
            var["category"] = cat->serialize();
            var["result"] = result.serialize();
            reply.process_data(var);
        }

        EXPECT_TRUE(received_result != nullptr);
        EXPECT_FALSE(received_result->direct_activation());
        EXPECT_EQ("scope-foo", received_result->activation_scope_name());

        // simulate aggregator scope
        std::shared_ptr<CategorisedResult> agg_received_result;
        auto aggreceiver = std::make_shared<DummyReceiver>([&agg_received_result](CategorisedResult result)
                {
                    agg_received_result.reset(new CategorisedResult(result));
                });
        internal::ResultReplyObject aggreply(aggreceiver, runtime.get(), "scope-bar");
        aggreply.set_disconnect_function(df);

        {
            CategorisedResult outerresult(cat);
            outerresult.set_uri("http://ubuntu.com/2");
            outerresult.set_dnd_uri("http://canonical.com/2");
            outerresult.store(*received_result, false);
            outerresult.set_intercept_activation();

            // push category and result through ResultReplyObject
            VariantMap var;
            var["category"] = cat->serialize();
            var["result"] = outerresult.serialize();
            aggreply.process_data(var);
        }

        EXPECT_TRUE(agg_received_result != nullptr);
        EXPECT_TRUE(agg_received_result->has_stored_result());
        EXPECT_FALSE(agg_received_result->direct_activation());
        // scope changed to agg scope because it intercepts activation
        EXPECT_EQ("scope-bar", agg_received_result->activation_scope_name());
    }
}

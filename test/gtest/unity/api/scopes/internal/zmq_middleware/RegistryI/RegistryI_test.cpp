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
 * along with this program.  If not, see <http://www.gnu.org/lzmqnses/>.
 *
 * Authored by: Michi Henning <michi.henning@canonical.com>
 */

#include <scopes/internal/zmq_middleware/RegistryI.h>

#include <scopes/internal/RegistryConfig.h>
#include <scopes/internal/RuntimeImpl.h>
#include <scopes/internal/UniqueID.h>
#include <internal/zmq_middleware/capnproto/Message.capnp.h>
#include <scopes/ScopeExceptions.h>
#include <unity/UnityExceptions.h>

#include <gtest/gtest.h>
#include <scope-api-testconfig.h>

#include <cassert>
#include <set>

using namespace std;
using namespace unity;
using namespace unity::api::scopes;
using namespace unity::api::scopes::internal;
using namespace unity::api::scopes::internal::zmq_middleware;

TEST(RegistryI, find)
{
try
{
    RuntimeImpl::UPtr runtime = RuntimeImpl::create(
        "Registry", TEST_BUILD_ROOT "/gtest/unity/api/scopes/internal/zmq_middleware/RegistryI/Runtime.ini");

    string identity = runtime->registry_identity();
    RegistryConfig c(identity, runtime->registry_configfile());
    string mw_kind = c.mw_kind();
    string mw_endpoint = c.endpoint();
    string mw_configfile = c.mw_configfile();

    MiddlewareBase::SPtr middleware = runtime->factory()->create(identity, mw_kind, mw_configfile);
    RegistryObject::SPtr ro(make_shared<RegistryObject>());
    auto registry = middleware->add_registry_object(identity, ro);
    auto proxy = middleware->create_scope_proxy("scope1", "ipc:///tmp/scope1");
    EXPECT_TRUE(ro->add("scope1", proxy));

    auto r = runtime->registry();
    auto scope = r->find("scope1");
    EXPECT_TRUE(scope.get());
}
catch(unity::Exception const& e)
{
    cout << e.to_string() << endl;
}
}

TEST(RegistryI, list)
{
    RuntimeImpl::UPtr runtime = RuntimeImpl::create(
        "Registry", TEST_BUILD_ROOT "/gtest/unity/api/scopes/internal/zmq_middleware/RegistryI/Runtime.ini");

    string identity = runtime->registry_identity();
    RegistryConfig c(identity, runtime->registry_configfile());
    string mw_kind = c.mw_kind();
    string mw_endpoint = c.endpoint();
    string mw_configfile = c.mw_configfile();

    MiddlewareBase::SPtr middleware = runtime->factory()->create(identity, mw_kind, mw_configfile);
    RegistryObject::SPtr ro(make_shared<RegistryObject>());
    auto registry = middleware->add_registry_object(identity, ro);

    auto r = runtime->registry();
    auto scopes = r->list();
    EXPECT_TRUE(scopes.empty());

    auto proxy = middleware->create_scope_proxy("scope1", "ipc:///tmp/scope1");
    EXPECT_TRUE(ro->add("scope1", proxy));
    scopes = r->list();
    EXPECT_EQ(1, scopes.size());
    EXPECT_NE(scopes.end(), scopes.find("scope1"));

    ro->remove("scope1");
    scopes = r->list();
    EXPECT_EQ(0, scopes.size());

    set<string> ids;
    for (int i = 0; i < 1000; ++i)
    {
        string long_id = "0000000000000000000000000000000000000000000000" + to_string(i);
        EXPECT_TRUE(ro->add(long_id, proxy));
        ids.insert(long_id);
    }
    scopes = r->list();
    EXPECT_EQ(1000, scopes.size());
    for (auto& id : ids)
    {
        auto it = scopes.find(id);
        EXPECT_NE(scopes.end(), it);
        EXPECT_NE(ids.end(), ids.find(it->first));
    }
}

TEST(RegistryI, add_remove)
{
    RuntimeImpl::UPtr runtime = RuntimeImpl::create(
        "Registry", TEST_BUILD_ROOT "/gtest/unity/api/scopes/internal/zmq_middleware/RegistryI/Runtime.ini");

    string identity = runtime->registry_identity();
    RegistryConfig c(identity, runtime->registry_configfile());
    string mw_kind = c.mw_kind();
    string mw_endpoint = c.endpoint();
    string mw_configfile = c.mw_configfile();

    MiddlewareBase::SPtr middleware = runtime->factory()->create(identity, mw_kind, mw_configfile);
    RegistryObject::SPtr ro(make_shared<RegistryObject>());
    auto registry = middleware->add_registry_object(identity, ro);

    auto r = runtime->registry();
    auto scopes = r->list();
    EXPECT_TRUE(scopes.empty());

    auto proxy = middleware->create_scope_proxy("scope1", "ipc:///tmp/scope1");
    EXPECT_TRUE(ro->add("scope1", proxy));
    scopes = r->list();
    EXPECT_EQ(1, scopes.size());
    EXPECT_NE(scopes.end(), scopes.find("scope1"));
    EXPECT_FALSE(ro->add("scope1", proxy));

    EXPECT_TRUE(ro->remove("scope1"));
    scopes = r->list();
    EXPECT_EQ(0, scopes.size());
    EXPECT_FALSE(ro->remove("scope1"));

    set<string> ids;
    for (int i = 0; i < 1000; ++i)
    {
        string long_id = "0000000000000000000000000000000000000000000000" + to_string(i);
        ro->add(long_id, proxy);
        ids.insert(long_id);
    }
    scopes = r->list();
    EXPECT_EQ(1000, scopes.size());
    for (auto& id : ids)
    {
        auto it = scopes.find(id);
        EXPECT_NE(scopes.end(), it);
        EXPECT_NE(ids.end(), ids.find(it->first));
    }
}

TEST(RegistryI, exceptions)
{
    RuntimeImpl::UPtr runtime = RuntimeImpl::create(
        "Registry", TEST_BUILD_ROOT "/gtest/unity/api/scopes/internal/zmq_middleware/RegistryI/Runtime.ini");

    string identity = runtime->registry_identity();
    RegistryConfig c(identity, runtime->registry_configfile());
    string mw_kind = c.mw_kind();
    string mw_endpoint = c.endpoint();
    string mw_configfile = c.mw_configfile();

    MiddlewareBase::SPtr middleware = runtime->factory()->create(identity, mw_kind, mw_configfile);
    RegistryObject::SPtr ro(make_shared<RegistryObject>());
    auto registry = middleware->add_registry_object(identity, ro);
    auto proxy = middleware->create_scope_proxy("scope1", "ipc:///tmp/scope1");
    ro->add("scope1", proxy);

    auto r = runtime->registry();

    try
    {
        r->find("fred");
        FAIL();
    }
    catch (NotFoundException const& e)
    {
        EXPECT_EQ("unity::api::scopes::NotFoundException: Registry::find(): no such scope (name = fred)",
                  e.to_string());
    }

    try
    {
        r->find("");
        FAIL();
    }
    catch (MiddlewareException const& e)
    {
        EXPECT_EQ("unity::api::scopes::MiddlewareException: unity::InvalidArgumentException: "
                  "Registry: Cannot search for scope with empty name",
                  e.to_string());
    }

    try
    {
        auto proxy = middleware->create_scope_proxy("scope1", "ipc:///tmp/scope1");
        ro->add("", proxy);
        FAIL();
    }
    catch (InvalidArgumentException const& e)
    {
        EXPECT_EQ("unity::InvalidArgumentException: Registry: Cannot add scope with empty name",
                  e.to_string());
    }

    try
    {
        ro->remove("");
        FAIL();
    }
    catch (InvalidArgumentException const& e)
    {
        EXPECT_EQ("unity::InvalidArgumentException: Registry: Cannot remove scope with empty name",
                  e.to_string());
    }
}

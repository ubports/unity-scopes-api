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
#include <scopes/internal/ScopeMetadataImpl.h>
#include <scopes/internal/ScopeImpl.h>
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
using namespace unity::scopes;
using namespace unity::scopes::internal;
using namespace unity::scopes::internal::zmq_middleware;

ScopeMetadata make_meta(const string& name, MWScopeProxy const& proxy, MiddlewareBase::SPtr const& mw)
{
    unique_ptr<ScopeMetadataImpl> mi(new ScopeMetadataImpl(mw.get()));
    mi->set_scope_name(name);
    mi->set_art("art " + name);
    mi->set_display_name("display name " + name);
    mi->set_description("description " + name);
    mi->set_search_hint("search hint " + name);
    mi->set_hot_key("hot key " + name);
    ScopeProxy p = ScopeImpl::create(proxy, mw->runtime());
    mi->set_proxy(p);
    return ScopeMetadataImpl::create(move(mi));
}

TEST(RegistryI, get_metadata)
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
        auto p = middleware->create_scope_proxy("scope1", "ipc:///tmp/scope1");
        EXPECT_TRUE(ro->add("scope1", move(make_meta("scope1", p, middleware))));

        auto r = runtime->registry();
        auto scope = r->get_metadata("scope1");
        EXPECT_EQ("scope1", scope.scope_name());
    }
    catch (unity::Exception const& e)
    {
        cerr << e.to_string() << endl;
        FAIL();
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
    EXPECT_TRUE(ro->add("scope1", move(make_meta("scope1", proxy, middleware))));
    scopes = r->list();
    EXPECT_EQ(1, scopes.size());
    EXPECT_NE(scopes.end(), scopes.find("scope1"));

    ro->remove("scope1");
    scopes = r->list();
    EXPECT_EQ(0, scopes.size());

    set<string> ids;
    for (int i = 0; i < 10; ++i)
    {
        string long_id = "0000000000000000000000000000000000000000000000" + to_string(i);
        EXPECT_TRUE(ro->add(long_id, move(make_meta(long_id, proxy, middleware))));
        ids.insert(long_id);
    }
    scopes = r->list();
    EXPECT_EQ(10, scopes.size());
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
    EXPECT_TRUE(ro->add("scope1", move(make_meta("scope1", proxy, middleware))));
    scopes = r->list();
    EXPECT_EQ(1, scopes.size());
    EXPECT_NE(scopes.end(), scopes.find("scope1"));
    EXPECT_FALSE(ro->add("scope1", move(make_meta("scope1", proxy, middleware))));

    EXPECT_TRUE(ro->remove("scope1"));
    scopes = r->list();
    EXPECT_EQ(0, scopes.size());
    EXPECT_FALSE(ro->remove("scope1"));

    set<string> ids;
    for (int i = 0; i < 10; ++i)
    {
        string long_id = "0000000000000000000000000000000000000000000000" + to_string(i);
        ro->add(long_id, move(make_meta(long_id, proxy, middleware)));
        ids.insert(long_id);
    }
    scopes = r->list();
    EXPECT_EQ(10, scopes.size());
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
    ro->add("scope1", move(make_meta("scope1", proxy, middleware)));

    auto r = runtime->registry();

    try
    {
        r->get_metadata("fred");
        FAIL();
    }
    catch (NotFoundException const& e)
    {
        EXPECT_EQ("unity::api::scopes::NotFoundException: Registry::get_metadata(): no such scope (name = fred)",
                  e.to_string());
    }

    try
    {
        r->get_metadata("");
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
        ro->add("", move(make_meta("blah", proxy, middleware)));
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

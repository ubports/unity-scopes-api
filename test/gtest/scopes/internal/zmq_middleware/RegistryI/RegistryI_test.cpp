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

#include <unity/scopes/internal/zmq_middleware/RegistryI.h>

#include <unity/scopes/internal/RegistryConfig.h>
#include <unity/scopes/internal/RegistryException.h>
#include <unity/scopes/internal/RegistryObject.h>
#include <unity/scopes/internal/RuntimeImpl.h>
#include <unity/scopes/internal/ScopeMetadataImpl.h>
#include <unity/scopes/internal/ScopeImpl.h>
#include <unity/scopes/internal/UniqueID.h>
#include <unity/scopes/internal/zmq_middleware/ZmqRegistry.h>
#include <scopes/internal/zmq_middleware/capnproto/Message.capnp.h>
#include <unity/scopes/ScopeExceptions.h>
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
    mi->set_scope_id(name);
    mi->set_art("art " + name);
    mi->set_display_name("display name " + name);
    mi->set_description("description " + name);
    mi->set_author("author " + name);
    mi->set_search_hint("search hint " + name);
    mi->set_hot_key("hot key " + name);
    ScopeProxy p = ScopeImpl::create(proxy, mw->runtime(), name);
    mi->set_proxy(p);
    return ScopeMetadataImpl::create(move(mi));
}

TEST(RegistryI, get_metadata)
{
    vector<string> dummy_spawn_command;
    RuntimeImpl::UPtr runtime = RuntimeImpl::create(
        "TestRegistry", TEST_BUILD_ROOT "/gtest/scopes/internal/zmq_middleware/RegistryI/Runtime.ini");

    string identity = runtime->registry_identity();
    RegistryConfig c(identity, runtime->registry_configfile());
    string mw_kind = c.mw_kind();
    string mw_endpoint = c.endpoint();
    string mw_configfile = c.mw_configfile();

    MiddlewareBase::SPtr middleware = runtime->factory()->create(identity, mw_kind, mw_configfile);
    RegistryObject::SPtr ro(make_shared<RegistryObject>());
    auto registry = middleware->add_registry_object(identity, ro);
    auto p = middleware->create_scope_proxy("scope1", "ipc:///tmp/scope1");
    EXPECT_TRUE(ro->add_local_scope("scope1", move(make_meta("scope1", p, middleware)),
            dummy_spawn_command));

    auto r = runtime->registry();
    auto scope = r->get_metadata("scope1");
    EXPECT_EQ("scope1", scope.scope_id());
}

TEST(RegistryI, list)
{
    RuntimeImpl::UPtr runtime = RuntimeImpl::create(
        "TestRegistry", TEST_BUILD_ROOT "/gtest/scopes/internal/zmq_middleware/RegistryI/Runtime.ini");

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

    vector<string> dummy_spawn_command;
    auto proxy = middleware->create_scope_proxy("scope1", "ipc:///tmp/scope1");
    EXPECT_TRUE(ro->add_local_scope("scope1", move(make_meta("scope1", proxy, middleware)),
            dummy_spawn_command));
    scopes = r->list();
    EXPECT_EQ(1u, scopes.size());
    EXPECT_NE(scopes.end(), scopes.find("scope1"));

    ro->remove_local_scope("scope1");
    scopes = r->list();
    EXPECT_EQ(0u, scopes.size());

    set<string> ids;
    for (int i = 0; i < 10; ++i)
    {
        string long_id = "0000000000000000000000000000000000000000000000" + to_string(i);
        EXPECT_TRUE(ro->add_local_scope(long_id, move(make_meta(long_id, proxy, middleware)),
                dummy_spawn_command));
        ids.insert(long_id);
    }
    scopes = r->list();
    EXPECT_EQ(10u, scopes.size());
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
        "TestRegistry", TEST_BUILD_ROOT "/gtest/scopes/internal/zmq_middleware/RegistryI/Runtime.ini");

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

    vector<string> dummy_spawn_command;
    auto proxy = middleware->create_scope_proxy("scope1", "ipc:///tmp/scope1");
    EXPECT_TRUE(ro->add_local_scope("scope1", move(make_meta("scope1", proxy, middleware)),
            dummy_spawn_command));
    scopes = r->list();
    EXPECT_EQ(1u, scopes.size());
    EXPECT_NE(scopes.end(), scopes.find("scope1"));
    EXPECT_FALSE(ro->add_local_scope("scope1", move(make_meta("scope1", proxy, middleware)),
            dummy_spawn_command));

    EXPECT_TRUE(ro->remove_local_scope("scope1"));
    scopes = r->list();
    EXPECT_EQ(0u, scopes.size());
    EXPECT_FALSE(ro->remove_local_scope("scope1"));

    set<string> ids;
    for (int i = 0; i < 10; ++i)
    {
        string long_id = "0000000000000000000000000000000000000000000000" + to_string(i);
        ro->add_local_scope(long_id, move(make_meta(long_id, proxy, middleware)),
                dummy_spawn_command);
        ids.insert(long_id);
    }
    scopes = r->list();
    EXPECT_EQ(10u, scopes.size());
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
        "TestRegistry", TEST_BUILD_ROOT "/gtest/scopes/internal/zmq_middleware/RegistryI/Runtime.ini");

    string identity = runtime->registry_identity();
    RegistryConfig c(identity, runtime->registry_configfile());
    string mw_kind = c.mw_kind();
    string mw_endpoint = c.endpoint();
    string mw_configfile = c.mw_configfile();

    MiddlewareBase::SPtr middleware = runtime->factory()->create(identity, mw_kind, mw_configfile);
    RegistryObject::SPtr ro(make_shared<RegistryObject>());
    vector<string> dummy_spawn_command;
    auto registry = middleware->add_registry_object(identity, ro);
    auto proxy = middleware->create_scope_proxy("scope1", "ipc:///tmp/scope1");
    ro->add_local_scope("scope1", move(make_meta("scope1", proxy, middleware)), dummy_spawn_command);

    auto r = runtime->registry();

    try
    {
        r->get_metadata("fred");
        FAIL();
    }
    catch (NotFoundException const& e)
    {
        EXPECT_STREQ("unity::scopes::NotFoundException: Registry::get_metadata(): no such scope (name = fred)",
                     e.what());
    }

    try
    {
        r->get_metadata("");
        FAIL();
    }
    catch (MiddlewareException const& e)
    {
        EXPECT_STREQ("unity::scopes::MiddlewareException: unity::InvalidArgumentException: "
                     "Registry: Cannot search for scope with empty name",
                     e.what());
    }

    try
    {
        auto proxy = middleware->create_scope_proxy("scope1", "ipc:///tmp/scope1");
        ro->add_local_scope("", move(make_meta("blah", proxy, middleware)), dummy_spawn_command);
        FAIL();
    }
    catch (InvalidArgumentException const& e)
    {
        EXPECT_STREQ("unity::InvalidArgumentException: Registry: Cannot add scope with empty name",
                     e.what());
    }

    try
    {
        ro->remove_local_scope("");
        FAIL();
    }
    catch (InvalidArgumentException const& e)
    {
        EXPECT_STREQ("unity::InvalidArgumentException: Registry: Cannot remove scope with empty name",
                     e.what());
    }
}

// RegistryObject that overrides the locate() method, so we can test it without having to run
// a full registry that spawns new processes.

class MockRegistryObject : public RegistryObject
{
public:
    MockRegistryObject()
    {
    }

    virtual ScopeProxy locate(string const& scope_name) override
    {
        if (scope_name == "no_such_scope")
        {
            throw NotFoundException("no can find", scope_name);
        }
        if (scope_name == "error_scope")
        {
            throw RegistryException("Couldn't start error_scope");
        }
        return get_metadata(scope_name).proxy();
    }
};

TEST(RegistryI, locate)
{
    RuntimeImpl::UPtr runtime = RuntimeImpl::create(
        "TestRegistry", TEST_BUILD_ROOT "/gtest/scopes/internal/zmq_middleware/RegistryI/Runtime.ini");

    string identity = runtime->registry_identity();
    RegistryConfig c(identity, runtime->registry_configfile());
    string mw_kind = c.mw_kind();
    string mw_endpoint = c.endpoint();
    string mw_configfile = c.mw_configfile();
    vector<string> dummy_spawn_command;

    MiddlewareBase::SPtr middleware = runtime->factory()->create(identity, mw_kind, mw_configfile);
    MockRegistryObject::SPtr mro(make_shared<MockRegistryObject>());
    auto r = middleware->add_registry_object(identity, mro);
    auto r_proxy = dynamic_pointer_cast<ZmqRegistry>(r);
    auto proxy = middleware->create_scope_proxy("scope1", "ipc:///tmp/scope1");
    mro->add_local_scope("scope1", move(make_meta("scope1", proxy, middleware)),
            dummy_spawn_command);

    auto p = r_proxy->locate("scope1");
    EXPECT_EQ("scope1", p->identity());
    EXPECT_EQ("ipc:///tmp/scope1", p->endpoint());

    try
    {
        r_proxy->locate("no_such_scope");
        FAIL();
    }
    catch (NotFoundException const& e)
    {
        EXPECT_STREQ("unity::scopes::NotFoundException: Registry::locate(): no such scope (name = no_such_scope)",
                     e.what());
    }

    try
    {
        r_proxy->locate("error_scope");
        FAIL();
    }
    catch (RegistryException const& e)
    {
        EXPECT_STREQ("unity::scopes::RegistryException: Couldn't start error_scope", e.what());
    }

    try
    {
        auto proxy = middleware->create_scope_proxy("scope1", "ipc:///tmp/scope1");
        mro->add_local_scope("", move(make_meta("blah", proxy, middleware)),
                 dummy_spawn_command);
        FAIL();
    }
    catch (InvalidArgumentException const& e)
    {
        EXPECT_STREQ("unity::InvalidArgumentException: Registry: Cannot add scope with empty name",
                     e.what());
    }

    try
    {
        mro->remove_local_scope("");
        FAIL();
    }
    catch (InvalidArgumentException const& e)
    {
        EXPECT_STREQ("unity::InvalidArgumentException: Registry: Cannot remove scope with empty name",
                     e.what());
    }
}

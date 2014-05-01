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

#include <scopes/internal/zmq_middleware/capnproto/Message.capnp.h>
#include <unity/scopes/CategorisedResult.h>
#include <unity/scopes/internal/RegistryConfig.h>
#include <unity/scopes/internal/RegistryException.h>
#include <unity/scopes/internal/RegistryObject.h>
#include <unity/scopes/internal/RuntimeImpl.h>
#include <unity/scopes/internal/ScopeMetadataImpl.h>
#include <unity/scopes/internal/ScopeImpl.h>
#include <unity/scopes/internal/UniqueID.h>
#include <unity/scopes/internal/zmq_middleware/ZmqRegistry.h>
#include <unity/scopes/SearchMetadata.h>
#include <unity/scopes/ScopeExceptions.h>
#include <unity/UnityExceptions.h>

#include <gtest/gtest.h>
#include <scope-api-testconfig.h>

#include <cassert>
#include <set>

using namespace std;
using namespace testing;
using namespace unity;
using namespace unity::scopes;
using namespace unity::scopes::internal;
using namespace unity::scopes::internal::zmq_middleware;

namespace
{
struct Scope
{
    Scope()
        : trap(core::posix::trap_signals_for_all_subsequent_threads({core::posix::Signal::sig_chld})),
          death_observer(core::posix::ChildProcess::DeathObserver::create_once_with_signal_trap(trap)),
          worker([this]() { trap->run(); })
    {
    }

    ~Scope()
    {
        trap->stop();

        if (worker.joinable())
            worker.join();
    }

    std::shared_ptr<core::posix::SignalTrap> trap;
    std::unique_ptr<core::posix::ChildProcess::DeathObserver> death_observer;
    std::thread worker;
} scope;

ScopeMetadata make_meta(const string& scope_id, MWScopeProxy const& proxy, MiddlewareBase::SPtr const& mw)
{
    unique_ptr<ScopeMetadataImpl> mi(new ScopeMetadataImpl(mw.get()));

    mi->set_scope_id(scope_id);
    mi->set_art("art " + scope_id);
    mi->set_display_name("display name " + scope_id);
    mi->set_description("description " + scope_id);
    mi->set_author("author " + scope_id);
    mi->set_search_hint("search hint " + scope_id);
    mi->set_hot_key("hot key " + scope_id);
    mi->set_scope_directory("/foo");
    ScopeProxy p = ScopeImpl::create(proxy, mw->runtime(), scope_id);
    mi->set_proxy(p);
    return ScopeMetadataImpl::create(move(mi));
}
}

TEST(RegistryI, get_metadata)
{
    RegistryObject::ScopeExecData dummy_exec_data;
    RuntimeImpl::UPtr runtime = RuntimeImpl::create(
        "TestRegistry", TEST_BUILD_ROOT "/gtest/scopes/internal/zmq_middleware/RegistryI/Runtime.ini");

    string identity = runtime->registry_identity();
    RegistryConfig c(identity, runtime->registry_configfile());
    string mw_kind = c.mw_kind();
    string mw_configfile = c.mw_configfile();

    MiddlewareBase::SPtr middleware = runtime->factory()->create(identity, mw_kind, mw_configfile);
    Executor::SPtr executor = make_shared<Executor>();
    RegistryObject::SPtr ro(make_shared<RegistryObject>(*scope.death_observer, executor));
    auto registry = middleware->add_registry_object(identity, ro);
    auto p = middleware->create_scope_proxy("scope1", "ipc:///tmp/scope1");
    EXPECT_TRUE(ro->add_local_scope("scope1", move(make_meta("scope1", p, middleware)),
            dummy_exec_data));

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
    string mw_configfile = c.mw_configfile();

    MiddlewareBase::SPtr middleware = runtime->factory()->create(identity, mw_kind, mw_configfile);
    Executor::SPtr executor = make_shared<Executor>();
    RegistryObject::SPtr ro(make_shared<RegistryObject>(*scope.death_observer, executor));
    auto registry = middleware->add_registry_object(identity, ro);

    auto r = runtime->registry();
    auto scopes = r->list();
    EXPECT_TRUE(scopes.empty());

    RegistryObject::ScopeExecData dummy_exec_data;
    auto proxy = middleware->create_scope_proxy("scope1", "ipc:///tmp/scope1");
    EXPECT_TRUE(ro->add_local_scope("scope1", move(make_meta("scope1", proxy, middleware)),
            dummy_exec_data));
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
                dummy_exec_data));
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
    string mw_configfile = c.mw_configfile();

    MiddlewareBase::SPtr middleware = runtime->factory()->create(identity, mw_kind, mw_configfile);
    Executor::SPtr executor = make_shared<Executor>();
    RegistryObject::SPtr ro(make_shared<RegistryObject>(*scope.death_observer, executor));
    auto registry = middleware->add_registry_object(identity, ro);

    auto r = runtime->registry();
    auto scopes = r->list();
    EXPECT_TRUE(scopes.empty());

    RegistryObject::ScopeExecData dummy_exec_data;
    auto proxy = middleware->create_scope_proxy("scope1", "ipc:///tmp/scope1");
    EXPECT_TRUE(ro->add_local_scope("scope1", move(make_meta("scope1", proxy, middleware)),
            dummy_exec_data));
    scopes = r->list();
    EXPECT_EQ(1u, scopes.size());
    EXPECT_NE(scopes.end(), scopes.find("scope1"));
    EXPECT_FALSE(ro->add_local_scope("scope1", move(make_meta("scope1", proxy, middleware)),
            dummy_exec_data));

    EXPECT_TRUE(ro->remove_local_scope("scope1"));
    scopes = r->list();
    EXPECT_EQ(0u, scopes.size());
    EXPECT_FALSE(ro->remove_local_scope("scope1"));

    set<string> ids;
    for (int i = 0; i < 10; ++i)
    {
        string long_id = "0000000000000000000000000000000000000000000000" + to_string(i);
        ro->add_local_scope(long_id, move(make_meta(long_id, proxy, middleware)),
                dummy_exec_data);
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
    string mw_configfile = c.mw_configfile();

    MiddlewareBase::SPtr middleware = runtime->factory()->create(identity, mw_kind, mw_configfile);
    Executor::SPtr executor = make_shared<Executor>();
    RegistryObject::SPtr ro(make_shared<RegistryObject>(*scope.death_observer, executor));
    RegistryObject::ScopeExecData dummy_exec_data;
    auto registry = middleware->add_registry_object(identity, ro);
    auto proxy = middleware->create_scope_proxy("scope1", "ipc:///tmp/scope1");
    ro->add_local_scope("scope1", move(make_meta("scope1", proxy, middleware)), dummy_exec_data);

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
                     "RegistryObject::get_metadata(): Cannot search for scope with empty id",
                     e.what());
    }

    try
    {
        auto proxy = middleware->create_scope_proxy("scope1", "ipc:///tmp/scope1");
        ro->add_local_scope("", move(make_meta("blah", proxy, middleware)), dummy_exec_data);
        FAIL();
    }
    catch (InvalidArgumentException const& e)
    {
        EXPECT_STREQ("unity::InvalidArgumentException: RegistryObject::add_local_scope(): Cannot add scope with empty id",
                     e.what());
    }

    try
    {
        ro->remove_local_scope("");
        FAIL();
    }
    catch (InvalidArgumentException const& e)
    {
        EXPECT_STREQ("unity::InvalidArgumentException: RegistryObject::remove_local_scope(): Cannot remove scope with empty id",
                     e.what());
    }
}

// RegistryObject that overrides the locate() method, so we can test it without having to run
// a full registry that spawns new processes.

class MockRegistryObject : public RegistryObject
{
public:
    MockRegistryObject(core::posix::ChildProcess::DeathObserver& death_observer)
        : RegistryObject(death_observer, make_shared<Executor>())
    {
    }

    virtual ObjectProxy locate(string const& identity) override
    {
        if (identity == "no_such_scope")
        {
            throw NotFoundException("no can find", identity);
        }
        if (identity == "error_scope")
        {
            throw RegistryException("Couldn't start error_scope");
        }
        return get_metadata(identity).proxy();
    }
};

TEST(RegistryI, locate_mock)
{
    RuntimeImpl::UPtr runtime = RuntimeImpl::create(
        "TestRegistry", TEST_BUILD_ROOT "/gtest/scopes/internal/zmq_middleware/RegistryI/Runtime.ini");

    string identity = runtime->registry_identity();
    RegistryConfig c(identity, runtime->registry_configfile());
    string mw_kind = c.mw_kind();
    string mw_configfile = c.mw_configfile();
    RegistryObject::ScopeExecData dummy_exec_data;

    MiddlewareBase::SPtr middleware = runtime->factory()->create(identity, mw_kind, mw_configfile);
    MockRegistryObject::SPtr mro(make_shared<MockRegistryObject>(*scope.death_observer));
    auto r = middleware->add_registry_object(identity, mro);
    auto r_proxy = dynamic_pointer_cast<ZmqRegistry>(r);
    auto proxy = middleware->create_scope_proxy("scope1", "ipc:///tmp/scope1");
    mro->add_local_scope("scope1", move(make_meta("scope1", proxy, middleware)),
            dummy_exec_data);

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
        EXPECT_STREQ("unity::scopes::NotFoundException: Registry::locate(): no such object (name = no_such_scope)",
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
                 dummy_exec_data);
        FAIL();
    }
    catch (InvalidArgumentException const& e)
    {
        EXPECT_STREQ("unity::InvalidArgumentException: RegistryObject::add_local_scope(): Cannot add scope with empty id",
                     e.what());
    }

    try
    {
        mro->remove_local_scope("");
        FAIL();
    }
    catch (InvalidArgumentException const& e)
    {
        EXPECT_STREQ("unity::InvalidArgumentException: RegistryObject::remove_local_scope(): Cannot remove scope with empty id",
                     e.what());
    }
}

class RegistryTest : public Test
{
public:
    RegistryTest()
    {
        start_process_count = process_count();

        // configure registry
        rt_config = TEST_BUILD_ROOT "/gtest/scopes/internal/zmq_middleware/RegistryI/Runtime.ini";

        rt = RuntimeImpl::create("TestRegistry", rt_config);
        std::string reg_id = rt->registry_identity();

        RegistryConfig c(reg_id, rt->registry_configfile());
        std::string mw_kind = c.mw_kind();
        std::string scoperunner_path = c.scoperunner_path();

        mw = rt->factory()->find(reg_id, mw_kind);

        Executor::SPtr executor = make_shared<Executor>();
        reg = RegistryObject::SPtr(new RegistryObject(*scope.death_observer, executor));
        mw->add_registry_object(reg_id, reg);
        mw->add_state_receiver_object("StateReceiver", reg->state_receiver());

        // configure scopes
        scope_ids = { {"scope-A", "scope-B", "scope-C", "scope-D", "scope-N", "scope-S"} };
        for (auto& scope_id : scope_ids)
        {
            proxies[scope_id] = ScopeImpl::create(mw->create_scope_proxy(scope_id), mw->runtime(), scope_id);

            unique_ptr<ScopeMetadataImpl> mi(new ScopeMetadataImpl(mw.get()));
            mi->set_scope_id(scope_id);
            mi->set_display_name(scope_id);
            mi->set_description(scope_id);
            mi->set_author("Canonical Ltd.");
            mi->set_proxy(proxies[scope_id]);
            mi->set_scope_directory("/foo");
            auto meta = ScopeMetadataImpl::create(move(mi));

            RegistryObject::ScopeExecData exec_data;
            exec_data.scope_id = scope_id;
            exec_data.scoperunner_path = scoperunner_path;
            exec_data.runtime_config = rt_config;
            exec_data.scope_config = TEST_BUILD_ROOT "/../demo/scopes/" + scope_id + "/" + scope_id + ".ini";

            reg->add_local_scope(scope_id, move(meta), exec_data);
        }
    }

    int first_child_pid()
    {
        return stoi(exec_cmd("ps --ppid " + std::to_string(getpid()) + " --no-headers"));
    }

    int process_count()
    {
        int current_process_count = stoi(exec_cmd("ps --ppid " + std::to_string(getpid()) + " | wc -l"));
        return current_process_count - start_process_count;
    }

protected:
    std::string exec_cmd(std::string const& cmd)
    {
        FILE* pipe = popen(cmd.c_str(), "r");
        if (!pipe)
        {
            return "";
        }

        char buffer[128];
        std::string result;
        while (!feof(pipe))
        {
            if (fgets(buffer, 128, pipe) != NULL)
            {
                result += buffer;
            }
        }
        pclose(pipe);

        return result;
    }

    int start_process_count = 0;
    std::string rt_config;
    RuntimeImpl::UPtr rt;
    MiddlewareBase::SPtr mw;
    RegistryObject::SPtr reg;
    std::array<std::string, 6> scope_ids;
    std::map<std::string, ScopeProxy> proxies;
};

// test initial state
TEST_F(RegistryTest, locate_init)
{
    // check that no scope processes are running
    for (auto const& scope_id : scope_ids)
    {
        EXPECT_FALSE(reg->is_scope_running(scope_id));
    }

    // check that no new processes have been started yet
    EXPECT_EQ(0, process_count());
}

// test locating the same scope multiple times
TEST_F(RegistryTest, locate_one)
{
    // locate all scopes (hence starting all scope processes)
    for (auto const& scope_id : scope_ids)
    {
        EXPECT_EQ(proxies[scope_id], reg->locate(scope_id));
    }

    // check that 6 new processes were started
    EXPECT_EQ(6, process_count());

    // locate the second scope multiple times
    for (int i = 0; i < 1000; ++i)
    {
        EXPECT_EQ(proxies[scope_ids[1]], reg->locate(scope_ids[1]));
    }

    // check that no new processes were started
    EXPECT_EQ(6, process_count());
}

// test locating all scopes
TEST_F(RegistryTest, locate_all)
{
    // locate all scopes (hence starting all scope processes)
    for (auto const& scope_id : scope_ids)
    {
        EXPECT_EQ(proxies[scope_id], reg->locate(scope_id));
    }

    // check that all scopes processes are running
    for (auto const& scope_id : scope_ids)
    {
        EXPECT_TRUE(reg->is_scope_running(scope_id));
    }

    // check that 6 new processes were started
    EXPECT_EQ(6, process_count());
}

class Receiver : public SearchListenerBase
{
public:
    void push(CategorisedResult) override {}

    void finished(Reason, std::string const&) override
    {
        // Signal that the query is complete
        unique_lock<std::mutex> lock(mutex_);
        query_complete_ = true;
        cond_.notify_one();
    }

    void wait_until_finished()
    {
        unique_lock<std::mutex> lock(mutex_);
        cond_.wait(lock, [this] { return this->query_complete_; });
    }

private:
    bool query_complete_;
    mutex mutex_;
    condition_variable cond_;
};

// test scope death and rebinding
TEST_F(RegistryTest, locate_rebinding)
{
    // locate first scope
    EXPECT_EQ(proxies[scope_ids[0]], reg->locate(scope_ids[0]));

    // check that the first scope is running
    EXPECT_TRUE(reg->is_scope_running(scope_ids[0]));

    // check that 1 new process was started
    EXPECT_EQ(1, process_count());

    // kill first scope
    int scope1_pid = first_child_pid();
    kill(scope1_pid, SIGKILL);

    // wait for the SIGCHLD signal to reach the registry
    while (reg->is_scope_running(scope_ids[0]))
    {
        std::this_thread::sleep_for(std::chrono::milliseconds{10});
    }

    // check that we now have no running scopes
    EXPECT_EQ(0, process_count());

    // locate first scope indirectly via rebinding on scope request
    auto receiver = make_shared<Receiver>();
    EXPECT_NO_THROW(proxies[scope_ids[0]]->search("test", SearchMetadata("en", "phone"), receiver));
    receiver->wait_until_finished();

    // check that the first scope is running
    EXPECT_TRUE(reg->is_scope_running(scope_ids[0]));

    // check that 1 new process was started
    EXPECT_EQ(1, process_count());
}

// test removing a scope
TEST_F(RegistryTest, locate_remove)
{
    // locate all scopes (hence starting all scope processes)
    for (auto const& scope_id : scope_ids)
    {
        EXPECT_EQ(proxies[scope_id], reg->locate(scope_id));
    }

    // check that 6 new processes were started
    EXPECT_EQ(6, process_count());

    // remove a scope (hence killing the process)
    EXPECT_TRUE(reg->remove_local_scope(scope_ids[0]));

    // check that we now have 5 scopes running
    EXPECT_EQ(5, process_count());

    // when the registry is destroyed all scope processes are killed
    reg.reset();
    mw.reset();
    rt.reset();

    // check that we are back to the original number of processes
    EXPECT_EQ(0, process_count());
}

// test custom scoperunner executable
TEST_F(RegistryTest, locate_custom_exec)
{
    std::string test_scope_id = "testscopeB";
    ScopeProxy test_proxy = ScopeImpl::create(mw->create_scope_proxy(test_scope_id), mw->runtime(), test_scope_id);

    unique_ptr<ScopeMetadataImpl> mi(new ScopeMetadataImpl(mw.get()));
    mi->set_scope_id(test_scope_id);
    mi->set_display_name(test_scope_id);
    mi->set_description(test_scope_id);
    mi->set_author("Canonical Ltd.");
    mi->set_proxy(test_proxy);
    mi->set_scope_directory("/foo");
    auto meta = ScopeMetadataImpl::create(move(mi));

    RegistryObject::ScopeExecData exec_data;
    exec_data.scope_id = test_scope_id;
    exec_data.scoperunner_path = TEST_BUILD_ROOT "/gtest/scopes/Registry/scopes/testscopeB/testscopeB";
    exec_data.runtime_config = rt_config;
    exec_data.scope_config = TEST_BUILD_ROOT "/gtest/scopes/Registry/scopes/testscopeB/testscopeB.ini";

    reg->add_local_scope(test_scope_id, move(meta), exec_data);

    // check that no new processes have been started yet
    EXPECT_EQ(0, process_count());

    // locate scope
    EXPECT_EQ(test_proxy, reg->locate(test_scope_id));

    // check that the scope is running
    EXPECT_TRUE(reg->is_scope_running(test_scope_id));

    // check that 1 new process was started
    EXPECT_EQ(1, process_count());

    // kill scope
    int scope1_pid = first_child_pid();
    kill(scope1_pid, SIGKILL);

    // wait for the SIGCHLD signal to reach the registry
    while (reg->is_scope_running(test_scope_id))
    {
        std::this_thread::sleep_for(std::chrono::milliseconds{10});
    }

    // check that we now have no running scopes
    EXPECT_EQ(0, process_count());
}

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
 * Authored by: Marcus Tomlinson <marcus.tomlinson@canonical.com>
 */

#include <unity/scopes/ChildScope.h>

#include <unity/scopes/internal/RegistryObject.h>
#include <unity/scopes/internal/ScopeImpl.h>

#include "TestScope.h"

#include <gtest/gtest.h>

using namespace testing;
using namespace unity::scopes;
using namespace unity::scopes::internal;

class ChildScopesTest : public Test
{
public:
    ChildScopesTest()
    {
        // Run a test registry
        trap_ = core::posix::trap_signals_for_all_subsequent_threads({core::posix::Signal::sig_chld});
        death_observer_ = core::posix::ChildProcess::DeathObserver::create_once_with_signal_trap(trap_);
        reg_rt_ = RuntimeImpl::create("TestRegistry", "Runtime.ini");
        auto reg_mw = reg_rt_->factory()->create("TestRegistry", "Zmq", "Zmq.ini");
        auto reg_obj(std::make_shared<RegistryObject>(*death_observer_, std::make_shared<Executor>(), reg_mw));
        reg_mw->add_registry_object("TestRegistry", reg_obj);

        // Create a proxy to TestScope
        scope_rt_ = RuntimeImpl::create("", "Runtime.ini");
        auto scope_mw = scope_rt_->factory()->create("TestScope", "Zmq", "Zmq.ini");
        scope_mw->start();
        auto proxy = scope_mw->create_scope_proxy("TestScope");
        test_scope = ScopeImpl::create(proxy, scope_rt_.get(), "TestScope");
    }

protected:
    ScopeProxy test_scope;

private:
    std::shared_ptr<core::posix::SignalTrap> trap_;
    std::unique_ptr<core::posix::ChildProcess::DeathObserver> death_observer_;
    RuntimeImpl::UPtr reg_rt_;
    RuntimeImpl::UPtr scope_rt_;
};

TEST_F(ChildScopesTest, basic)
{
    ChildScopeList list;
    list.push_back({"ScopeA", true});
    list.push_back({"ScopeB", false});
    list.push_back({"ScopeC", false});

    test_scope->set_child_scopes_ordered(list);

    ChildScopeList return_list = test_scope->child_scopes_ordered();
    EXPECT_EQ(3, return_list.size());
    EXPECT_EQ("ScopeA", return_list.front().id);
}

int main(int argc, char **argv)
{
    InitGoogleTest(&argc, argv);

    // Run TestScope in a separate thread
    auto rt = Runtime::create_scope_runtime("TestScope", "Runtime.ini");
    std::thread scope_t([&rt]
    {
        TestScope scope;
        rt->run_scope(&scope, "");
    });

    // Give the scope some time to bind to its endpoint
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // Run tests
    int rc = RUN_ALL_TESTS();

    // Clean up
    rt->destroy();
    scope_t.join();

    return rc;
}

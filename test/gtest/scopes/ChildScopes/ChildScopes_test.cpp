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

#include <unity/scopes/internal/RegistryObject.h>
#include <unity/scopes/internal/ScopeImpl.h>

#include "TestScope.h"

#include <gtest/gtest.h>

using namespace testing;
using namespace unity::scopes;
using namespace unity::scopes::internal;

std::shared_ptr<core::posix::SignalTrap> trap(core::posix::trap_signals_for_all_subsequent_threads({core::posix::Signal::sig_chld}));
std::unique_ptr<core::posix::ChildProcess::DeathObserver> death_observer(core::posix::ChildProcess::DeathObserver::create_once_with_signal_trap(trap));

class ChildScopesTest : public Test
{
public:
    ChildScopesTest()
    {
        // Run a test registry
        reg_rt_ = RuntimeImpl::create("TestRegistry", "Runtime.ini");
        auto mw = reg_rt_->factory()->create("TestRegistry", "Zmq", "Zmq.ini");
        auto reg_obj(std::make_shared<RegistryObject>(*death_observer, std::make_shared<Executor>(), mw));
        mw->add_registry_object("TestRegistry", reg_obj);
    }

private:
    RuntimeImpl::SPtr reg_rt_;
};

TEST_F(ChildScopesTest, basic)
{
    auto rt = RuntimeImpl::create("", "Runtime.ini");
    auto mw = rt->factory()->create("TestScope", "Zmq", "Zmq.ini");
    mw->start();
    auto proxy = mw->create_scope_proxy("TestScope");
    auto scope = ScopeImpl::create(proxy, rt.get(), "TestScope");
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    // Run TestScope in a separate thread
    auto rt = Runtime::create_scope_runtime("TestScope", "Runtime.ini");
    std::thread scope_t([&rt]
    {
        TestScope scope;
        rt->run_scope(&scope, "");
    });

    // Give threads some time to bind to their endpoints
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // Run tests
    int rc = RUN_ALL_TESTS();

    // Clean up
    rt->destroy();
    scope_t.join();

    return rc;
}

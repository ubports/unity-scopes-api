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
 * Authored by: Michi Henning <michi.henning@canonical.com>
 */

#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

#include <scopes/CategorisedResult.h>
#include <scopes/ReceiverBase.h>
#include <scopes/Runtime.h>
#include <scopes/internal/RuntimeImpl.h>
#include <scopes/internal/MWScope.h>
#include <scopes/internal/ScopeImpl.h>
#include <unity/UnityExceptions.h>

#include <gtest/gtest.h>

using namespace std;
using namespace unity::api::scopes;

TEST(Runtime, basic)
{
    Runtime::UPtr rt = Runtime::create("Runtime.ini");
    EXPECT_TRUE(rt->registry().get() != nullptr);
    rt->destroy();
}

class Receiver : public ReceiverBase
{
public:
    virtual void push(CategorisedResult) override
    {
    }
    virtual void finished(ReceiverBase::Reason) override
    {
    }
};

TEST(Runtime, run_scope)
{
    // Spawn the test scope
    const char *const argv[] = {"./Runtime_TestScope", "Runtime.ini", NULL};
    pid_t pid;
    switch (pid = fork()) {
    case -1:
        FAIL();
    case 0: // child
        execv(argv[0], (char *const *)argv);
        FAIL();
    }

    // Parent
    try{

    auto rt = internal::RuntimeImpl::create("", "Runtime.ini");
    auto mw = rt->factory()->create("TestScope", "Zmq", "Zmq.ini");
    auto proxy = mw->create_scope_proxy("TestScope");
    auto scope = internal::ScopeImpl::create(proxy, rt.get());

    VariantMap hints;
    auto receiver = make_shared<Receiver>();
    auto ctrl = scope->create_query("test", hints, receiver);

    } catch (unity::Exception const &e) {
        cerr << e.to_string() << std::endl;
        throw;
    }

    kill(pid, SIGTERM);
}

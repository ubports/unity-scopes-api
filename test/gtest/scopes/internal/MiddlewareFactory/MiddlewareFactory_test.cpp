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

#include <unity/scopes/internal/MiddlewareFactory.h>
#include <unity/scopes/ScopeExceptions.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <gtest/gtest.h>
#pragma GCC diagnostic pop

using namespace std;
using namespace unity::scopes;
using namespace unity::scopes::internal;

TEST(MiddlewareFactory, basic)
{
    MiddlewareFactory f(reinterpret_cast<RuntimeImpl*>(0x1));
    EXPECT_EQ(nullptr, f.find("nosuchscope", "Zmq"));
    MiddlewareBase::SPtr not_found;
    MiddlewareBase::SPtr mw = f.find("testscope", "Zmq");
    EXPECT_EQ(not_found, mw);
    mw = f.find("testscope", "NoSuchMiddleware");
    EXPECT_EQ(not_found, mw);
}

TEST(MiddlewareFactory, BadKind)
{
    try
    {
        MiddlewareFactory f(reinterpret_cast<RuntimeImpl*>(0x1));
        f.create("somescope", "NoSuchMiddleware", "Zmq.ini");
        FAIL();
    }
    catch (ConfigException const& e)
    {
        EXPECT_STREQ("unity::scopes::ConfigException: Invalid middleware kind: NoSuchMiddleware", e.what());
    }
}

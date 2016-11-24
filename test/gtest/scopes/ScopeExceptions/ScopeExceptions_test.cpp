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

#include <unity/scopes/ScopeExceptions.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <gtest/gtest.h>
#pragma GCC diagnostic pop

using namespace std;
using namespace unity::scopes;

TEST(MiddlewareException, state)
{
    {
        MiddlewareException e("some error");
        EXPECT_STREQ("unity::scopes::MiddlewareException: some error", e.what());
        EXPECT_THROW(rethrow_exception(e.self()), MiddlewareException);
        MiddlewareException e2("blah");
        e2 = e;
        EXPECT_EQ(e.reason(), e2.reason());
    }
}

TEST(ConfigException, state)
{
    {
        ConfigException e("some error");
        EXPECT_STREQ("unity::scopes::ConfigException: some error", e.what());
        EXPECT_THROW(rethrow_exception(e.self()), ConfigException);
        ConfigException e2("blah");
        e2 = e;
        EXPECT_EQ(e.reason(), e2.reason());
    }
}

TEST(NotFoundException, state)
{
    {
        NotFoundException e("some error", "name");
        EXPECT_STREQ("unity::scopes::NotFoundException: some error (name = name)", e.what());
        EXPECT_EQ("name", e.name());
        EXPECT_THROW(rethrow_exception(e.self()), NotFoundException);
        NotFoundException e2("blah", "name");
        e2 = e;
        EXPECT_EQ(e.reason(), e2.reason());
        EXPECT_EQ(e.name(), e2.name());
    }
}

TEST(ObjectNotExistException, state)
{
    {
        ObjectNotExistException e("some error", "some id");
        EXPECT_STREQ("unity::scopes::ObjectNotExistException: some error (id = some id)", e.what());
        EXPECT_EQ("unity::scopes::ObjectNotExistException", e.name());
        EXPECT_EQ("some id", e.id());
        EXPECT_THROW(rethrow_exception(e.self()), ObjectNotExistException);
        ObjectNotExistException e2("blah", "some id");
        e2 = e;
        EXPECT_EQ(e.reason(), e2.reason());
        EXPECT_EQ(e.name(), e2.name());
    }
}

TEST(TimeoutException, state)
{
    {
        TimeoutException e("some error");
        EXPECT_STREQ("unity::scopes::TimeoutException: some error", e.what());
        EXPECT_EQ("unity::scopes::TimeoutException", e.name());
        EXPECT_THROW(rethrow_exception(e.self()), TimeoutException);
        TimeoutException e2("blah");
        e2 = e;
        EXPECT_EQ(e.reason(), e2.reason());
        EXPECT_EQ(e.name(), e2.name());
    }
}

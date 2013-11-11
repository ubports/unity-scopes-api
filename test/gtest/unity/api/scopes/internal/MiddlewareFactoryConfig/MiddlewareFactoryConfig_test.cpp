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

#include <scopes/internal/MiddlewareFactoryConfig.h>

#include <gtest/gtest.h>

using namespace std;
using namespace unity::api::scopes::internal;

TEST(MiddlewareFactoryConfig, basic)
{
    MiddlewareFactoryConfig c("Factory.ini");
    EXPECT_EQ("Ice.Config", c.ice_configfile());
    EXPECT_EQ("REST.Config", c.rest_configfile());
}

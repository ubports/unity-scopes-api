/*
 * Copyright (C) 2013 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Michi Henning <michi.henning@canonical.com>
 */

#include <unity/api/scopes/internal/ice_middleware/IceMiddleware.h>

#include <gtest/gtest.h>
#include <scope-api-testconfig.h>

using namespace unity::api::scopes::internal::ice_middleware;

// Basic test.

TEST(IceMiddleware, basic)
{
    IceMiddleware im("testscope", TEST_BUILD_ROOT "/gtest/unity/api/scopes/internal/ice_middleware/Ice.config");
    im.start();
    sleep(1);
    im.stop();
}

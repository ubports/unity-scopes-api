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

#include <scopes/internal/zmq_middleware/ZmqMiddleware.h>

#include <gtest/gtest.h>
#include <scope-api-testconfig.h>

using namespace std;
using namespace unity::api::scopes::internal;
using namespace unity::api::scopes::internal::zmq_middleware;

// Basic test.

TEST(ZmqMiddleware, basic)
{
    ZmqMiddleware im("testscope",
                     TEST_BUILD_ROOT "/gtest/unity/api/scopes/internal/zmq_middleware/ZmqMiddleware/Zmq.ini",
                     (RuntimeImpl*)0x1);
    im.start();
    sleep(1);
    im.stop();
}

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

#include <unity/scopes/internal/zmq_middleware/Util.h>

#include <boost/regex.hpp>  // Use Boost implementation until http://gcc.gnu.org/bugzilla/show_bug.cgi?id=53631 is fixed.
#include <unity/scopes/ScopeExceptions.h>

#include <sys/un.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <gtest/gtest.h>
#pragma GCC diagnostic pop

using namespace std;
using namespace unity;
using namespace unity::scopes;
using namespace unity::scopes::internal::zmq_middleware;

TEST(Util, basic)
{
    struct sockaddr_un addr;
    auto const maxlen = sizeof(addr.sun_path) - 1;

    const string schema = "ipc://";
    const string short_endpoint = schema + "/tmp/blah";
    const string max_endpoint = schema + string(maxlen, 'x');
    const string max_endpoint_plus_one = schema + string(maxlen + 1, 'x');

    throw_if_bad_endpoint(short_endpoint);         // Must not throw
    throw_if_bad_endpoint(max_endpoint);           // Must throw
    try
    {
        throw_if_bad_endpoint(max_endpoint_plus_one);  // Must throw
        FAIL();
    }
    catch (MiddlewareException const& e)
    {
        boost::regex r("unity::scopes::MiddlewareException: Socket path name too long \\(max = [0-9]+\\)\\: x+");
        EXPECT_TRUE(boost::regex_match(e.what(), r));
    }
}

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

#include <unity/scopes/ScopeExceptions.h>

#include <sys/un.h>

using namespace std;

namespace unity
{

namespace scopes
{

namespace internal
{

namespace zmq_middleware
{

// Make sure we get a meaningful error message if a UNIX domain
// socket has a path that exceeds the maximum length.

void throw_if_bad_endpoint(string const& endpoint)
{
    const std::string transport_prefix = "ipc://";
    if (endpoint.substr(0, transport_prefix.length()) != transport_prefix)
    {
        return;  // We only check paths starting with "ipc://"
    }
    string path = endpoint.substr(transport_prefix.length());
    struct sockaddr_un addr;
    auto const maxlen = sizeof(addr.sun_path) - 1;
    if (path.length() > maxlen)
    {
        throw MiddlewareException("Socket path name too long (max = " + to_string(maxlen) + "): " + path);
    }
}

} // namespace zmq_middleware

} // namespace internal

} // namespace scopes

} // namespace unity

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
#include <unity/util/ResourcePtr.h>

#include <unistd.h>
#include <sys/socket.h>
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

// For the ipc transport, zmq permits more than one server to bind to the same endpoint.
// If a server binds to an endpoint while another server is using that endpoint, the
// second server silently "steals" the endpoint from the previous server, so all
// connects after that point go to the new server, while connects that happened earlier
// go to the old server. This is meant as a fail-over feature, and cannot be disabled.
//
// We don't want this and need an error if two servers try to use the same endpoint.
// Hacky solution: we check whether it's possible to successfully connect to the
// endpoint. If so, a server is still running there, and we throw. This has a
// small race because a second server may connect after the check, but before
// the bind. But, in practice, that's good enough for our purposes.

void safe_bind(zmqpp::socket& s, string const& endpoint)
{
    const std::string transport_prefix = "ipc://";
    if (endpoint.substr(0, transport_prefix.size()) == transport_prefix)
    {
        string path = endpoint.substr(transport_prefix.size());
        struct sockaddr_un addr;
        memset(&addr, 0, sizeof(addr));
        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, path.c_str(), sizeof(addr.sun_path) - 1);
        int fd = ::socket(AF_UNIX, SOCK_STREAM, 0);
        if (fd == -1)
        {
            // LCOV_EXCL_START
            throw MiddlewareException("safe_bind(): cannot create socket: " + std::string(strerror(errno)));
            // LCOV_EXCL_STOP
        }
        util::ResourcePtr<int, decltype(&::close)> close_guard(fd, ::close);
        if (::connect(fd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) == 0)
        {
            // Connect succeeded, so another server is using the socket already.
            throw MiddlewareException("safe_bind(): address in use: " + endpoint);
        }
    }
    s.bind(endpoint);
}

} // namespace zmq_middleware

} // namespace internal

} // namespace scopes

} // namespace unity

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

#include <unity/scopes/internal/zmq_middleware/ZmqSender.h>

#include <cassert>

using namespace std;

namespace unity
{

namespace scopes
{

namespace internal
{

namespace zmq_middleware
{

ZmqSender::ZmqSender(zmqpp::socket& s) :
    s_(s)
{
}

// Send a message provided as a capnp segment list. Each segment is sent as a separate zmq message part.

void ZmqSender::send(kj::ArrayPtr<kj::ArrayPtr<capnp::word const> const> segments)
{
    auto it = segments.begin();
    auto i = segments.size();
    assert(i != 0);
    while (--i != 0)
    {
        s_.send_raw(reinterpret_cast<char const*>(&(*it)[0]), it->size() * sizeof(capnp::word), zmqpp::socket::send_more);
        ++it;
    }
    s_.send_raw(reinterpret_cast<char const*>(&(*it)[0]), it->size() * sizeof(capnp::word), zmqpp::socket::normal);
}

} // namespace zmq_middleware

} // namespace internal

} // namespace scopes

} // namespace unity

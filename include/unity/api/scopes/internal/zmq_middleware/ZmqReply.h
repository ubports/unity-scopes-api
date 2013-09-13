/*
 * Copyright (C) 2013 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the Lesser GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * Lesser GNU General Public License for more details.
 *
 * You should have received a copy of the Lesser GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Michi Henning <michi.henning@canonical.com>
 */

#ifndef UNITY_API_SCOPES_INTERNAL_ZMQMIDDLEWARE_ZMQREPLY_H
#define UNITY_API_SCOPES_INTERNAL_ZMQMIDDLEWARE_ZMQREPLY_H

#include <unity/api/scopes/internal/zmq_middleware/ZmqObjectProxy.h>
#include <unity/api/scopes/internal/zmq_middleware/ZmqReplyProxyFwd.h>
#include <unity/api/scopes/internal/MWReply.h>

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

namespace zmq_middleware
{

class ZmqReply : public virtual ZmqObjectProxy, public virtual MWReply
{
public:
    ZmqReply(ZmqMiddleware* mw_base, std::string const& endpoint, std::string const& identity);
    virtual ~ZmqReply() noexcept;

    virtual void push(std::string const& result) override;
    virtual void finished() override;
};

} // namespace zmq_middleware

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

#endif

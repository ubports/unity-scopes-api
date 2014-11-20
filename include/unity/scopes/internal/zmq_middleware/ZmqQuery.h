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

#pragma once

#include <unity/scopes/internal/zmq_middleware/ZmqObjectProxy.h>
#include <unity/scopes/internal/zmq_middleware/ZmqQueryProxyFwd.h>
#include <unity/scopes/internal/MWQuery.h>

namespace unity
{

namespace scopes
{

namespace internal
{

namespace zmq_middleware
{

class ZmqQuery : public virtual ZmqObjectProxy, public virtual MWQuery
{
public:
    ZmqQuery(ZmqMiddleware* mw_base,
             std::string const& endpoint,
             std::string const& identity,
             std::string const& category);
    virtual ~ZmqQuery();

    virtual void run(MWReplyProxy const& reply) override;
};

} // namespace zmq_middleware

} // namespace internal

} // namespace scopes

} // namespace unity

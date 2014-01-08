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

#ifndef UNITY_API_SCOPES_INTERNAL_ZMQMIDDLEWARE_ZMQSCOPE_H
#define UNITY_API_SCOPES_INTERNAL_ZMQMIDDLEWARE_ZMQSCOPE_H

#include <scopes/internal/zmq_middleware/ZmqObjectProxy.h>
#include <scopes/internal/zmq_middleware/ZmqScopeProxyFwd.h>
#include <scopes/internal/MWScope.h>

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

class ZmqScope : public virtual ZmqObjectProxy, public virtual MWScope
{
public:
    ZmqScope(ZmqMiddleware* mw_base, std::string const& endpoint, std::string const& identity);
    virtual ~ZmqScope() noexcept;

    virtual QueryCtrlProxy create_query(std::string const& q,
                                        VariantMap const& hints,
                                        MWReplyProxy const& reply) override;
    virtual QueryCtrlProxy preview(Result const& result,
                                   VariantMap const& hints,
                                   MWReplyProxy const& reply) override;
};

} // namespace zmq_middleware

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

#endif

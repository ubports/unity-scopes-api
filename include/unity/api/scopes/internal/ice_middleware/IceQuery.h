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

#ifndef UNITY_API_SCOPES_INTERNAL_ICEMIDDLEWARE_ICEQUERY_H
#define UNITY_API_SCOPES_INTERNAL_ICEMIDDLEWARE_ICEQUERY_H

#include <slice/unity/api/scopes/internal/ice_middleware/Query.h>
#include <unity/api/scopes/internal/ice_middleware/IceObjectProxy.h>
#include <unity/api/scopes/internal/ice_middleware/IceQueryProxyFwd.h>
#include <unity/api/scopes/internal/MWQuery.h>

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

namespace ice_middleware
{

class IceQuery : public virtual IceObjectProxy, public virtual MWQuery
{
public:
    IceQuery(IceMiddleware* mw_base, middleware::QueryPrx const& p) noexcept;
    virtual ~IceQuery() noexcept;

    virtual void run(MWReplyProxy const& r) override;
};

} // namespace ice_middleware

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

#endif

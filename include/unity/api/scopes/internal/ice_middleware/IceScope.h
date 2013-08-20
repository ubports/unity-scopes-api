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

#ifndef UNITY_API_SCOPES_INTERNAL_ICEMIDDLEWARE_ICESCOPE_H
#define UNITY_API_SCOPES_INTERNAL_ICEMIDDLEWARE_ICESCOPE_H

#include <slice/unity/api/scopes/internal/ice_middleware/Scope.h>
#include <unity/api/scopes/internal/ice_middleware/IceObject.h>
#include <unity/api/scopes/internal/ice_middleware/IceScopeProxyFwd.h>
#include <unity/api/scopes/internal/MWScope.h>

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

class IceScope : public virtual IceObject, public virtual MWScope
{
public:
    IceScope(IceMiddleware* mw_base, middleware::ScopePrx const& p) noexcept;
    virtual ~IceScope() noexcept;

    virtual QueryCtrlProxy create_query(std::string const& q, MWReplyProxy const& reply) override;
};

} // namespace ice_middleware

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

#endif

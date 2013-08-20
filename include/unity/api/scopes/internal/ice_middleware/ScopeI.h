/*
 * Copyright (C) 2013 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Michi Henning <michi.henning@canonical.com>
 */

#ifndef UNITY_API_SCOPES_INTERNAL_ICEMIDDLEWARE_SCOPEI_H
#define UNITY_API_SCOPES_INTERNAL_ICEMIDDLEWARE_SCOPEI_H

#include <slice/unity/api/scopes/internal/ice_middleware/Scope.h>
#include <unity/api/scopes/internal/ScopeObject.h>

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

class IceMiddleware;

// Server-side implementation of a scope object.

class ScopeI : public middleware::Scope
{
public:
    ScopeI(IceMiddleware* mw_base, ScopeObject::SPtr const& ro);
    virtual ~ScopeI() noexcept;

    // Slice operations
    virtual middleware::QueryCtrlPrx createQuery(std::string const& q,
                                                 middleware::ReplyPrx const& r,
                                                 Ice::Current const&) override;

private:
    IceMiddleware* mw_base_;
    ScopeObject::SPtr so_;
};

typedef IceUtil::Handle<ScopeI> ScopeIPtr;

} // namespace ice_middleware

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

#endif

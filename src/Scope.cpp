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

#include <scopes/Scope.h>

#include <scopes/internal/ScopeImpl.h>

namespace unity
{

namespace api
{

namespace scopes
{

//! @cond

Scope::Scope(internal::ScopeImpl* impl) :
    ObjectProxy(impl)
{
}

Scope::~Scope() noexcept
{
}

QueryCtrlProxy Scope::create_query(std::string const& q, VariantMap const& hints, SearchListener::SPtr const& reply) const
{
    return fwd()->create_query(q, hints, reply);
}

internal::ScopeImpl* Scope::fwd() const
{
    return dynamic_cast<internal::ScopeImpl*>(pimpl());
}

//! @endcond

} // namespace scopes

} // namespace api

} // namespace unity

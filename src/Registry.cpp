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

#include <scopes/Registry.h>

#include <scopes/internal/RegistryImpl.h>

namespace unity
{

namespace api
{

namespace scopes
{

//! @cond

Registry::Registry(internal::RegistryImpl* impl) :
    ObjectProxy(impl)
{
}

Registry::~Registry() noexcept
{
}

//! @endcond

ScopeMetadata Registry::find(std::string const& scope_name) const
{
    return fwd()->find(scope_name);
}

ScopeMap Registry::list() const
{
    return fwd()->list();
}

ScopeMap Registry::list_if(std::function<bool(ScopeMetadata const& item)> predicate) const
{
    return fwd()->list_if(predicate);
}

internal::RegistryImpl* Registry::fwd() const
{
    return dynamic_cast<internal::RegistryImpl*>(pimpl());
}

} // namespace scopes

} // namespace api

} // namespace unity

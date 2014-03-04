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

#include <unity/scopes/internal/Registry.h>

#include <unity/scopes/internal/RegistryImpl.h>

namespace unity
{

namespace scopes
{

namespace internal
{

//! @cond

Registry::Registry(internal::RegistryImpl* impl) :
    ObjectProxy(impl)
{
}

Registry::~Registry()
{
}

//! @endcond

ScopeMetadata Registry::get_metadata(std::string const& scope_id) const
{
    return fwd()->get_metadata(scope_id);
}

MetadataMap Registry::list() const
{
    return fwd()->list();
}

MetadataMap Registry::list_if(std::function<bool(ScopeMetadata const& item)> predicate) const
{
    return fwd()->list_if(predicate);
}

internal::RegistryImpl* Registry::fwd() const
{
    return dynamic_cast<internal::RegistryImpl*>(pimpl());
}

} // namespace internal

} // namespace scopes

} // namespace unity

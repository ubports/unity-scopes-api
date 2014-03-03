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

#include <unity/scopes/internal/RegistryImpl.h>

#include <unity/scopes/internal/MWRegistry.h>
#include <unity/scopes/ScopeExceptions.h>
#include <unity/UnityExceptions.h>

using namespace std;

namespace unity
{

namespace scopes
{

namespace internal
{

RegistryImpl::RegistryImpl(MWRegistryProxy const& mw_proxy, RuntimeImpl*) :
    ObjectProxyImpl(mw_proxy)
{
}

RegistryImpl::~RegistryImpl()
{
}

ScopeMetadata RegistryImpl::get_metadata(std::string const& scope_id)
{
    return fwd()->get_metadata(scope_id);
}

MetadataMap RegistryImpl::list()
{
    return fwd()->list();
}

ScopeProxy RegistryImpl::locate(std::string const& scope_id)
{
    return fwd()->locate(scope_id);
}

MetadataMap RegistryImpl::list_if(std::function<bool(ScopeMetadata const& item)> predicate)
{
    auto scope_map = list();
    MetadataMap matching_entries;
    for (auto& pair : scope_map)
    {
        if (predicate(pair.second))
        {
            matching_entries.emplace(pair);
        }
    }
    return matching_entries;
}

RegistryProxy RegistryImpl::create(MWRegistryProxy const& mw_proxy, RuntimeImpl* runtime)
{
    return RegistryProxy(new Registry(new RegistryImpl(mw_proxy, runtime)));
}

MWRegistryProxy RegistryImpl::fwd() const
{
    return dynamic_pointer_cast<MWRegistry>(proxy());
}

} // namespace internal

} // namespace scopes

} // namespace unity

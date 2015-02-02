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
#include <unity/scopes/internal/RuntimeImpl.h>

using namespace std;

namespace unity
{

namespace scopes
{

namespace internal
{

RegistryImpl::RegistryImpl(MWRegistryProxy const& mw_proxy)
    : ObjectImpl(mw_proxy)
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

ObjectProxy RegistryImpl::locate(std::string const& identity)
{
    return fwd()->locate(identity);
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

bool RegistryImpl::is_scope_running(std::string const& scope_id)
{
    return fwd()->is_scope_running(scope_id);
}

core::ScopedConnection RegistryImpl::set_scope_state_callback(std::string const& scope_id, std::function<void(bool)> callback)
{
    return fwd()->set_scope_state_callback(scope_id, callback);
}

core::ScopedConnection RegistryImpl::set_list_update_callback(std::function<void()> callback)
{
    return fwd()->set_list_update_callback(callback);
}

MWRegistryProxy RegistryImpl::fwd()
{
    return dynamic_pointer_cast<MWRegistry>(proxy());
}

} // namespace internal

} // namespace scopes

} // namespace unity

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

#include <unity/scopes/internal/RegistryObject.h>

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

RegistryObject::RegistryObject()
{
}

RegistryObject::~RegistryObject()
{
}

ScopeMetadata RegistryObject::get_metadata(std::string const& scope_name) const
{
    lock_guard<decltype(mutex_)> lock(mutex_);
    // If the name is empty, it was sent as empty by the remote client.
    if (scope_name.empty())
    {
        throw unity::InvalidArgumentException("Registry: Cannot search for scope with empty name");
    }

    // Look for the scope in both the local and the remote map.
    // Local scopes take precedence over remote ones of the same
    // name. (Ideally, this will never happen, except maybe
    // during development.)
    auto const& it = scopes_.find(scope_name);
    if (it != scopes_.end())
    {
        return it->second;
    }

    if (remote_registry_)
    {
        return remote_registry_->get_metadata(scope_name);
    }

    throw NotFoundException("Registry::get_metadata(): no such scope",  scope_name);
}

MetadataMap RegistryObject::list() const
{
    lock_guard<decltype(mutex_)> lock(mutex_);
    MetadataMap all_scopes(scopes_);  // Local scopes

    // If a remote scope has the same name as a local one,
    // this will not overwrite a local scope with a remote
    // one if they have the same name.
    if (remote_registry_)
    {
        MetadataMap remote_scopes = remote_registry_->list();
        all_scopes.insert(remote_scopes.begin(), remote_scopes.end());
    }

    return all_scopes;
}

bool RegistryObject::add_local_scope(ScopeMetadata const& metadata, ScopeExecData const& exec_data)
{
    lock_guard<decltype(mutex_)> lock(mutex_);
    bool return_value = true;
    std::string scope_name = metadata.scope_name();
    if (scope_name.empty())
    {
        throw unity::InvalidArgumentException("Registry: Cannot add scope with empty name");
    }
    if(scope_name.find('/') != std::string::npos) {
        throw unity::InvalidArgumentException("Registry: Cannot create a scope with a slash in its name");
    }

    if (scopes_.find(scope_name) != scopes_.end())
    {
        scopes_.erase(scope_name);
        exec_datas_.erase(scope_name);
        return_value = false;
    }
    scopes_.insert(make_pair(scope_name, metadata));
    exec_datas_.insert(make_pair(scope_name, exec_data));
    return return_value;
}

bool RegistryObject::remove_local_scope(std::string const& scope_name)
{
    lock_guard<decltype(mutex_)> lock(mutex_);
    // If the name is empty, it was sent as empty by the remote client.
    if (scope_name.empty())
    {
        throw unity::InvalidArgumentException("Registry: Cannot remove scope with empty name");
    }

    exec_datas_.erase(scope_name);
    return scopes_.erase(scope_name) == 1;
}

void RegistryObject::set_remote_registry(MWRegistryProxy const& remote_registry)
{
    lock_guard<decltype(mutex_)> lock(mutex_);
    remote_registry_ = remote_registry;
}

ScopeProxy RegistryObject::locate(std::string const& scope_name)
{
    lock_guard<decltype(mutex_)> lock(mutex_);
    // If the name is empty, it was sent as empty by the remote client.
    if (scope_name.empty())
    {
        throw unity::InvalidArgumentException("Registry: Cannot locate scope with empty name");
    }

    auto it = scopes_.find(scope_name);
    if (it == scopes_.end())
    {
        throw NotFoundException("Tried to obtain unknown scope", scope_name);
    }

    return it->second.proxy();
}

} // namespace internal

} // namespace scopes

} // namespace unity

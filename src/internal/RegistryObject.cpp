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

#include <scopes/internal/RegistryObject.h>

#include <scopes/ScopeExceptions.h>
#include <unity/UnityExceptions.h>

using namespace std;

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

RegistryObject::RegistryObject()
{
}

RegistryObject::~RegistryObject() noexcept
{
}

ScopeMetadata RegistryObject::get_metadata(std::string const& scope_name)
{
    // If the name is empty, it was sent as empty by the remote client.
    if (scope_name.empty())
    {
        throw unity::InvalidArgumentException("Registry: Cannot search for scope with empty name");
    }

    lock_guard<decltype(mutex_)> lock(mutex_);

    auto const& it = scopes_.find(scope_name);
    if (it == scopes_.end())
    {
        throw NotFoundException("Registry::get_metadata(): no such scope",  scope_name);
    }
    return it->second;
}

MetadataMap RegistryObject::list()
{
    lock_guard<decltype(mutex_)> lock(mutex_);
    return scopes_;
}

bool RegistryObject::add(std::string const& scope_name, ScopeMetadata const& metadata)
{
    if (scope_name.empty())
    {
        throw unity::InvalidArgumentException("Registry: Cannot add scope with empty name");
    }
    // TODO: check for names containing a slash, because that won't work if we use
    //       the name for a socket in the file system.

    lock_guard<decltype(mutex_)> lock(mutex_);

    auto const& pair = scopes_.insert(make_pair(scope_name, metadata));
    if (!pair.second)
    {
        // Replace already existing entry with this one
        scopes_.erase(pair.first);
        scopes_.insert(make_pair(scope_name, metadata));
        return false;
    }
    return true;
}

ScopeProxy RegistryObject::locate(std::string const& scope_name)
{
    // If the name is empty, it was sent as empty by the remote client.
    if (scope_name.empty())
    {
        throw unity::InvalidArgumentException("Registry: Cannot locate scope with empty name");
    }

    // TODO: missing implementation. This should activate the scope if it isn't running already
    // and return the proxy for the scope. Failures such as not being able to exec the scope
    // need to raise RegistryException.
    throw NotFoundException("Registry::locate(): no such scope",  scope_name);
}

bool RegistryObject::remove(std::string const& scope_name)
{
    // If the name is empty, it was sent as empty by the remote client.
    if (scope_name.empty())
    {
        throw unity::InvalidArgumentException("Registry: Cannot remove scope with empty name");
    }

    lock_guard<decltype(mutex_)> lock(mutex_);

    return scopes_.erase(scope_name) == 1;
}

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

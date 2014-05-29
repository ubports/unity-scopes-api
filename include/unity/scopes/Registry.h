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

#ifndef UNITY_SCOPES_REGISTRY_H
#define UNITY_SCOPES_REGISTRY_H

#include <unity/scopes/Object.h>
#include <unity/scopes/RegistryProxyFwd.h>
#include <unity/scopes/ScopeMetadata.h>

#include <map>

namespace unity
{

namespace scopes
{

namespace internal
{
class RegistryImpl;
}

/**
\brief Map for scope ID and metadata pairs.
*/
typedef std::map<std::string, ScopeMetadata> MetadataMap;

/**
\brief White pages service for available scopes.

You can obtain a proxy to the registry by calling Runtime::registry().
*/

class Registry : public virtual Object
{
public:
    /// @cond
    virtual ~Registry();
    /// @endcond

    /**
    \brief Returns the metadata for the scope with the given ID.
    \return The metadata for the scope.
    \throws NotFoundException if no scope with the given name exists.
    */
    virtual ScopeMetadata get_metadata(std::string const& scope_id) = 0;

    /**
    \brief Returns a map containing the metadata for all scopes.
    \return The metadata for all scopes.
    */
    virtual MetadataMap list() = 0;

    /**
    \brief Returns a map containing only those scopes for which `predicate` returns true.
    \param predicate a function object that must return true for each metadata item to be included in the map.
    \return The metadata items for which the predicate returned true.
    */
    virtual MetadataMap list_if(std::function<bool(ScopeMetadata const& item)> predicate) = 0;

    /**
    \brief Assigns a callback method to be executed when the registry's scope list changes.

    Note: Upon receiving this callback, you should retrieve the updated scopes list via the list() method if
    you wish to retain synchronisation between client and server.
    \param callback The function object that is invoked when an update occurs.
    */
    virtual void set_list_update_callback(std::function<void()> callback) = 0;

protected:
    /// @cond
    Registry();
    /// @endcond
};

} // namespace scopes

} // namespace unity

#endif

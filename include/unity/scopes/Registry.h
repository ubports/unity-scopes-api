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

#include <unity/scopes/ObjectProxy.h>
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
\brief Map for scope name and metadata pairs.
*/
typedef std::map<std::string, ScopeMetadata> MetadataMap;

/**
\brief White pages service for available scopes.
You can obtain a proxy to the registry by calling Runtime::registry().
*/

class Registry : public virtual ObjectProxy
{
public:
    /// @cond
    virtual ~Registry();
    Registry(const Registry&) = delete;
    Registry(Registry&&) = delete;
    /// @endcond

    /**
    \brief Returns the metadata for the scope with the given id.
    \return The metadata for the scope.
    \throws NotFoundException if no scope with the given name exists.
    */
    virtual ScopeMetadata get_metadata(std::string const& scope_name) const = 0;

    /**
    \brief Returns a map containing the metadata for all scopes.
    \return The metadata for all scopes.
    */
    virtual MetadataMap list() const = 0;

    /**
    \brief Returns a map containing only those scopes for which `predicate` returns true.
    \param predicate a function object that must return true for each metadata item to be included in the map.
    \return The metadata items for which the predicate returned true.
    */
    virtual MetadataMap list_if(std::function<bool(ScopeMetadata const& item)> predicate) const = 0;

protected:
    Registry();
};

} // namespace scopes

} // namespace unity

#endif

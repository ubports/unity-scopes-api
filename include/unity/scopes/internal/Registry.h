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

#ifndef UNITY_SCOPES_INTERNAL_REGISTRY_H
#define UNITY_SCOPES_INTERNAL_REGISTRY_H

#include <unity/scopes/Registry.h>

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

/**
\brief White pages service for available scopes.
You can obtain a proxy to the registry by calling Runtime::registry().
*/

class Registry : public unity::scopes::Registry
{
public:
    /// @cond
    virtual ~Registry();
    /// @endcond

    /**
    \brief Returns the metadata for the scope with the given id.
    \return The metadata for the scope.
    \throws NotFoundException if no scope with the given name exists.
    */
    ScopeMetadata get_metadata(std::string const& scope_id) const override;

    /**
    \brief Returns a map containing the metadata for all scopes.
    \return The metadata for all scopes.
    */
    MetadataMap list() const override;

    /**
    \brief Returns a map containing only those scopes for which predicate returns true.
    \param predicate a function object the must return true for each metadata item to be include in the map.
    \return The metadata items for which the predicate returned true.
    */
    MetadataMap list_if(std::function<bool(ScopeMetadata const& item)> predicate) const override;

protected:
    /// @cond
    Registry(RegistryImpl* impl);          // Instantiated only by RegistryImpl
    friend class RegistryImpl;
    /// @endcond

private:
    RegistryImpl* fwd() const;
};

} // namespace internal

} // namespace scopes

} // namespace unity

#endif

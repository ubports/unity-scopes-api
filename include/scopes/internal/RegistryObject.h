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

#ifndef UNITY_API_SCOPES_INTERNAL_REGISTRYOBJECT_H
#define UNITY_API_SCOPES_INTERNAL_REGISTRYOBJECT_H

#include <scopes/internal/AbstractObject.h>
#include <scopes/Registry.h>
#include <scopes/ScopeProxyFwd.h>
#include <mutex>

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

class RegistryObjectPrivate;

// Maintains a map of <scope name, scope proxy> pairs.

class RegistryObject : public AbstractObject
{
public:
    UNITY_DEFINES_PTRS(RegistryObject);

    RegistryObject();
    virtual ~RegistryObject() noexcept;

    // Remote operation implementations
    ScopeMetadata get_metadata(std::string const& scope_name);
    MetadataMap list();
    ScopeProxy get_scope(std::string const& scope_name);
    virtual ScopeProxy locate(std::string const& scope_name);  // virtual so we can mock the method for testing
    bool add(std::string const& scope_name, ScopeMetadata const& scope,
             std::vector<std::string> const& spawn_command);
    bool remove(std::string const& scope_name);

private:
    RegistryObjectPrivate* p;
    mutable std::mutex mutex_;
};

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

#endif

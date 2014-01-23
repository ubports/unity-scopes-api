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

#ifndef UNITY_SCOPES_INTERNAL_REGISTRYOBJECT_H
#define UNITY_SCOPES_INTERNAL_REGISTRYOBJECT_H

#include <unity/scopes/internal/RegistryObjectBase.h>

#include <mutex>

namespace unity
{

namespace scopes
{

namespace internal
{

class RegistryObjectPrivate;

// Maintains a map of <scope name, scope proxy> pairs.

class RegistryObject : public RegistryObjectBase
{
public:
    UNITY_DEFINES_PTRS(RegistryObject);

    RegistryObject();
    virtual ~RegistryObject();

    // Remote operation implementations
    virtual ScopeMetadata get_metadata(std::string const& scope_name) override;
    virtual MetadataMap list() override;
    virtual ScopeProxy locate(std::string const& scope_name) override;

    bool add(std::string const& scope_name, ScopeMetadata const& scope,
             std::vector<std::string> const& spawn_command);
    bool remove(std::string const& scope_name);

private:
    RegistryObjectPrivate* p;
    mutable std::mutex mutex_;
};

} // namespace internal

} // namespace scopes

} // namespace unity

#endif

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
 * Authored by: Marcus Tomlinson <marcus.tomlinson@canonical.com>
 */

#ifndef UNITY_SCOPES_INTERNAL_SMARTSCOPES_SMARTREGISTRYOBJECT_H
#define UNITY_SCOPES_INTERNAL_SMARTSCOPES_SMARTREGISTRYOBJECT_H

#include <unity/scopes/internal/AbstractObject.h>
#include <unity/scopes/internal/smartscopes/SmartScopesClient.h>
#include <unity/scopes/Registry.h>

#include <mutex>

namespace unity
{

namespace scopes
{

namespace internal
{

namespace smartscopes
{

class SmartRegistryObject : public AbstractObject
{
public:
    UNITY_DEFINES_PTRS(SmartRegistryObject);

    SmartRegistryObject();
    virtual ~SmartRegistryObject() noexcept;

    ScopeMetadata get_metadata(std::string const& scope_name);
    MetadataMap list();

private:
    void get_remote_scopes();
    bool add(std::string const& scope_name, ScopeMetadata const& scope);

private:
    smartscopes::SmartScopesClient ssclient_;

    MetadataMap scopes_;
    std::mutex mutex_;
};

} // namespace smartscopes

} // namespace internal

} // namespace scopes

} // namespace unity

#endif // UNITY_SCOPES_INTERNAL_SMARTSCOPES_SMARTREGISTRYOBJECT_H

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

#ifndef UNITY_SCOPES_INTERNAL_REGISTRYIMPL_H
#define UNITY_SCOPES_INTERNAL_REGISTRYIMPL_H

#include <unity/scopes/internal/MWRegistryProxyFwd.h>
#include <unity/scopes/internal/ObjectProxyImpl.h>
#include <unity/scopes/internal/Registry.h>

namespace unity
{

namespace scopes
{

namespace internal
{

class RuntimeImpl;

class RegistryImpl : public virtual ObjectProxyImpl
{
public:
    RegistryImpl(MWRegistryProxy const& mw_proxy, RuntimeImpl* runtime);
    ~RegistryImpl();

    ScopeMetadata get_metadata(std::string const& scope_id);
    MetadataMap list();
    MetadataMap list_if(std::function<bool(ScopeMetadata const& item)> predicate);
    ScopeProxy locate(std::string const& scope_name);

    static RegistryProxy create(MWRegistryProxy const& mw_proxy, RuntimeImpl* runtime);

private:
    MWRegistryProxy fwd() const;
};

} // namespace internal

} // namespace scopes

} // namespace unity

#endif

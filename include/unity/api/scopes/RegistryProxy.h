/*
 * Copyright (C) 2013 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the Lesser GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * Lesser GNU General Public License for more details.
 *
 * You should have received a copy of the Lesser GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Michi Henning <michi.henning@canonical.com>
 */

#ifndef UNITY_API_SCOPES_REGISTRYPROXY_H
#define UNITY_API_SCOPES_REGISTRYPROXY_H

#include <unity/api/scopes/ScopeProxy.h>

#include <map>

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{
class RegistryProxyImpl;
}

/**
\brief Map for scope name and proxy pairs.
*/
typedef std::map<std::string, ScopeProxy::SPtr> ScopeMap;

/**
\brief RegistryProxy provides access to the available scopes.
You can obtain a proxy to the registry by calling Runtime::registry().
*/

class UNITY_API RegistryProxy : public ObjectProxy
{
public:
    /// @cond
    UNITY_DEFINES_PTRS(RegistryProxy);

    virtual ~RegistryProxy() noexcept;
    /// @endcond

    /**
    \brief Returns a ScopeProxy for the scope with the given name.
    @return If no scope with the given name exists, the returned `shared_ptr` stores a `nullptr`.
    */
    ScopeProxy::SPtr find(std::string const& scope_name) const;

    /**
    \brief Returns a map containing the name/proxy pairs for all available scopes.
    */
    ScopeMap list() const;

private:
    RegistryProxy(internal::RegistryProxyImpl* impl);          // Only instantiable by RegistryProxyImpl
    friend class internal::RegistryProxyImpl;

    std::shared_ptr<internal::RegistryProxyImpl> p;
};

} // namespace scopes

} // namespace api

} // namespace unity

#endif

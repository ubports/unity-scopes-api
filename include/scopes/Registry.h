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

#ifndef UNITY_API_SCOPES_REGISTRY_H
#define UNITY_API_SCOPES_REGISTRY_H

#include <scopes/ObjectProxy.h>
#include <scopes/RegistryProxyFwd.h>
#include <scopes/Scope.h>

#include <map>

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{
class RegistryImpl;
}

/**
\brief Map for scope name and proxy pairs.
*/
typedef std::map<std::string, ScopeProxy> ScopeMap;

/**
\brief RegistryProxy provides access to the available scopes.
You can obtain a proxy to the registry by calling Runtime::registry().
*/

class UNITY_API Registry : public virtual ObjectProxy
{
public:
    /// @cond
    virtual ~Registry() noexcept;
    /// @endcond

    /**
    \brief Returns a ScopeProxy for the scope with the given name.
    @return If no scope with the given name exists, find() throws NotFoundException.
    */
    ScopeProxy find(std::string const& scope_name) const;

    /**
    \brief Returns a map containing the name/proxy pairs for all available scopes.
    */
    ScopeMap list() const;

protected:
    Registry(internal::RegistryImpl* impl);          // Instantiated only by RegistryImpl
    friend class internal::RegistryImpl;

private:
    internal::RegistryImpl* fwd() const;
};

} // namespace scopes

} // namespace api

} // namespace unity

#endif

/*
 * Copyright (C) 2013 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Michi Henning <michi.henning@canonical.com>
 */

#include <unity/api/scopes/internal/RegistryImpl.h>

#include <unity/api/scopes/internal/MWRegistry.h>
#include <unity/api/scopes/ScopeExceptions.h>
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

RegistryImpl::RegistryImpl(MWRegistryProxy const& mw_proxy) :
    mw_proxy_(mw_proxy)
{
}

RegistryImpl::~RegistryImpl() noexcept
{
}

// We swallow middleware exceptions in the methods below because
// a scope cannot reasonably do anything to recover from those.

ScopeProxy RegistryImpl::find(std::string const& scope_name)
{
    ScopeProxy scope_proxy;
    try
    {
        scope_proxy = mw_proxy_->find(scope_name);
    }
    catch (MiddlewareException const& e)
    {
        throw ResourceException("Cannot access the registry");
    }
    return scope_proxy;
}

ScopeMap RegistryImpl::list()
{
    ScopeMap sm;
    try
    {
        sm = mw_proxy_->list();
    }
    catch (MiddlewareException const& e)
    {
        throw ResourceException("Cannot access the registry");
    }
    return sm;
}

RegistryProxy RegistryImpl::create(MWRegistryProxy const& mw_proxy)
{
    return RegistryProxy(new Registry(new RegistryImpl(mw_proxy)));
}

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

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

#include <unity/api/scopes/internal/RegistryProxyImpl.h>

#include <unity/api/scopes/internal/MWRegistryProxy.h>
#include <unity/api/scopes/ScopeExceptions.h>

using namespace std;

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

RegistryProxyImpl::RegistryProxyImpl(MWRegistryProxy::SPtr const& mw_proxy) :
    mw_proxy_(mw_proxy)
{
}

RegistryProxyImpl::~RegistryProxyImpl() noexcept
{
}

// We swallow middleware exceptions in the methods below because
// a scope cannot reasonably do anything to recover from those.

ScopeProxy::SPtr RegistryProxyImpl::find(std::string const& scope_name)
{
    ScopeProxy::SPtr scope_proxy;
    try
    {
        scope_proxy = mw_proxy_->find(scope_name);
    }
    catch (MiddlewareException const& e)
    {
        // TODO: log error
    }
    return scope_proxy;
}

ScopeMap RegistryProxyImpl::list()
{
    ScopeMap sm;
    try
    {
        sm = mw_proxy_->list();
    }
    catch (MiddlewareException const& e)
    {
        // TODO: log error
    }
    return sm;
}

RegistryProxy::SPtr RegistryProxyImpl::create(MWRegistryProxy::SPtr const& mw_proxy)
{
    return RegistryProxy::SPtr(new RegistryProxy(new RegistryProxyImpl(mw_proxy)));
}

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

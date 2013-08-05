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

#include <unity/api/scopes/internal/ice_middleware/IceRegistryProxy.h>

#include <unity/api/scopes/internal/ice_middleware/IceScopeProxy.h>
#include <unity/api/scopes/internal/ice_middleware/RethrowException.h>
#include <unity/api/scopes/internal/ScopeProxyImpl.h>

using namespace std;

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

namespace ice_middleware
{

IceRegistryProxy::IceRegistryProxy(MiddlewareBase* mw_base, Ice::ObjectPrx const& p) noexcept :
    MWRegistryProxy(mw_base),
    proxy_(middleware::RegistryPrx::uncheckedCast(p))
{
    assert(p);
}

IceRegistryProxy::~IceRegistryProxy() noexcept
{
}

ScopeProxy::SPtr IceRegistryProxy::find(std::string const& scope_name)
{
    ScopeProxy::SPtr sp;
    try
    {
        middleware::ScopePrx p = proxy_->find(scope_name);
        if (p)
        {
            MWScopeProxy::SPtr proxy(new IceScopeProxy(mw_base(), p));
            sp = ScopeProxyImpl::create(proxy);
        }
    }
    catch (Ice::Exception const& e)
    {
        rethrow_ice_ex(e);
    }
    return sp;
}

ScopeMap IceRegistryProxy::list()
{
    ScopeMap sm;
    try
    {
        middleware::ScopeDict sd;
        sd = proxy_->list();
        for (auto const& it : sd)
        {
            assert(it.second);
            IceScopeProxy::SPtr sp(new IceScopeProxy(mw_base(), it.second));
            ScopeProxy::SPtr proxy = ScopeProxyImpl::create(sp);
            sm[it.first] = proxy;
        }
    }
    catch (Ice::Exception const& e)
    {
        rethrow_ice_ex(e);
    }
    return sm;
}

} // namespace ice_middleware

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

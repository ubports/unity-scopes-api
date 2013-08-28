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

#include <unity/api/scopes/internal/ice_middleware/IceRegistry.h>

#include <unity/api/scopes/internal/ice_middleware/slice/Scope.h>
#include <unity/api/scopes/internal/ice_middleware/IceScope.h>
#include <unity/api/scopes/internal/ice_middleware/RethrowException.h>
#include <unity/api/scopes/internal/ScopeImpl.h>
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

namespace ice_middleware
{

IceRegistry::IceRegistry(IceMiddleware* mw_base, middleware::RegistryPrx const& p) noexcept :
    MWObjectProxy(mw_base),
    IceObjectProxy(mw_base, p),
    MWRegistry(mw_base)
{
    assert(p);
}

IceRegistry::~IceRegistry() noexcept
{
}

ScopeProxy IceRegistry::find(std::string const& scope_name)
{
    ScopeProxy sp;
    try
    {
        auto self = middleware::RegistryPrx::uncheckedCast(proxy());
        middleware::ScopePrx p = self->find(scope_name);
        assert(p);
        IceScopeProxy isp(new IceScope(mw_base(), p));
        sp = ScopeImpl::create(isp, mw_base()->runtime());
    }
    catch (middleware::NotFoundException const& e)
    {
        throw NotFoundException("Registry::find(): no such scope", e.scopeName);
    }
    catch (Ice::Exception const& e)
    {
        rethrow_ice_ex(e);
    }
    return sp;
}

ScopeMap IceRegistry::list()
{
    ScopeMap sm;
    try
    {
        auto self = middleware::RegistryPrx::uncheckedCast(proxy());
        middleware::ScopeDict sd;
        sd = self->list();
        for (auto const& it : sd)
        {
            assert(it.second);
            IceScopeProxy isp(new IceScope(mw_base(), it.second));
            ScopeProxy proxy = ScopeImpl::create(isp, mw_base()->runtime());
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

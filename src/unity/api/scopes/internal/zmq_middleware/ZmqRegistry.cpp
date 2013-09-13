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

#include <unity/api/scopes/internal/zmq_middleware/ZmqRegistry.h>

#include <unity/api/scopes/internal/zmq_middleware/ZmqScope.h>
#include <unity/api/scopes/internal/zmq_middleware/RethrowException.h>
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

namespace zmq_middleware
{

ZmqRegistry::ZmqRegistry(ZmqMiddleware* mw_base, string const& endpoint, string const& identity) :
    MWObjectProxy(mw_base),
    ZmqObjectProxy(mw_base, endpoint, identity),
    MWRegistry(mw_base)
{
}

ZmqRegistry::~ZmqRegistry() noexcept
{
}

ScopeProxy ZmqRegistry::find(std::string const& scope_name)
{
    ScopeProxy sp;
#if 0
    try
    {
        auto self = RegistryProxy::uncheckedCast(proxy());
        middleware::ScopePrx p = self->find(scope_name); // Remote invocation here.
        assert(p);
        ZmqScopeProxy isp(new ZmqScope(mw_base(), p));
        sp = ScopeImpl::create(isp, mw_base()->runtime());
    }
    catch (middleware::NotFoundException const& e)
    {
        throw NotFoundException("Registry::find(): no such scope", e.scopeName);
    }
    catch (Zmq::Exception const& e)
    {
        rethrow_zmq_ex(e);
    }
#endif
    return sp;
}

ScopeMap ZmqRegistry::list()
{
    ScopeMap sm;
#if 0
    try
    {
        auto self = middleware::RegistryPrx::uncheckedCast(proxy());
        middleware::ScopeDict sd;
        sd = self->list();
        for (auto const& it : sd)
        {
            assert(it.second);
            ZmqScopeProxy isp(new ZmqScope(mw_base(), it.second));
            ScopeProxy proxy = ScopeImpl::create(isp, mw_base()->runtime());
            sm[it.first] = proxy;
        }
    }
    catch (Zmq::Exception const& e)
    {
        rethrow_zmq_ex(e);
    }
#endif
    return sm;
}

} // namespace zmq_middleware

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

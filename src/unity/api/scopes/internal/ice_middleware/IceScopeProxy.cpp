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

#include <unity/api/scopes/internal/ice_middleware/IceScopeProxy.h>

#include <unity/api/scopes/internal/ice_middleware/IceReplyProxy.h>
#include <unity/api/scopes/internal/ice_middleware/RethrowException.h>

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

IceScopeProxy::IceScopeProxy(MiddlewareBase* mw_base, Ice::ObjectPrx const& p) noexcept :
    MWScopeProxy(mw_base),
    proxy_(middleware::ScopePrx::uncheckedCast(p)->ice_oneway())
{
    // No assert here, it's legal to create a nil proxy for a scope (for example, when RegistryProxy::find()
    // doesn't find the scope).
    // Note that we make the proxy oneway, so the caller isn't blocked if the scope is slow to respond.
}

IceScopeProxy::~IceScopeProxy() noexcept
{
}

void IceScopeProxy::query(std::string const& q, MWReplyProxy::SPtr const& reply)
{
    try
    {
        IceReplyProxy::SPtr ice_reply_proxy = dynamic_pointer_cast<IceReplyProxy>(reply);
        proxy_->query(q, ice_reply_proxy->proxy());
    }
    catch (Ice::Exception const& e)
    {
        rethrow_ice_ex(e);
    }
}

middleware::ScopePrx IceScopeProxy::proxy() const noexcept
{
    return proxy_;
}

} // namespace ice_middleware

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

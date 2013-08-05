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

IceReplyProxy::IceReplyProxy(MiddlewareBase* mw_base, Ice::ObjectPrx const& p) noexcept :
    MWReplyProxy(mw_base),
    proxy_(middleware::ReplyPrx::uncheckedCast(p)->ice_oneway())
{
    assert(p);
    // Note that we make the proxy oneway. That way, the calling scope isn't blocked if the receiving scope is slow.
}

IceReplyProxy::~IceReplyProxy() noexcept
{
}

void IceReplyProxy::send(string const& result)
{
    try
    {
        proxy_->send(result);
    }
    catch (Ice::Exception const& e)
    {
        rethrow_ice_ex(e);
    }
}

void IceReplyProxy::finished()
{
    try
    {
        proxy_->finished();
    }
    catch (Ice::Exception const& e)
    {
        rethrow_ice_ex(e);
    }
}

middleware::ReplyPrx IceReplyProxy::proxy() const noexcept
{
    return proxy_;
}

} // namespace ice_middleware

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

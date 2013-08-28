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

#include <unity/api/scopes/internal/ice_middleware/IceQuery.h>

#include <unity/api/scopes/internal/ice_middleware/slice/Reply.h>
#include <unity/api/scopes/internal/ice_middleware/IceReply.h>
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

IceQuery::IceQuery(IceMiddleware* mw_base, middleware::QueryPrx const& p) noexcept :
    MWObjectProxy(mw_base),
    IceObjectProxy(mw_base, p->ice_oneway()->ice_collocationOptimized(false)),
    MWQuery(mw_base)
{
    // We turn off collocation optimization so a oneway call to run() by the QueryCtrl is dispatched
    // in a separated thread. (Without this, because Query and QueryCtrl share a communicator, the
    // call will be dispatched collocation optimized, that is, as a direct synchronous twoway call.)
    assert(p);
}

IceQuery::~IceQuery() noexcept
{
}

void IceQuery::run(MWReplyProxy const& r)
{
    try
    {
        auto ice_r = dynamic_pointer_cast<IceReply>(r);
        auto reply_proxy = middleware::ReplyPrx::uncheckedCast(ice_r->proxy());
        auto self = middleware::QueryPrx::uncheckedCast(proxy());
        self->run(reply_proxy);
    }
    catch (Ice::Exception const& e)
    {
        rethrow_ice_ex(e);
    }
}

} // namespace ice_middleware

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

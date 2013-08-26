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

#include <unity/api/scopes/internal/ice_middleware/IceScope.h>

#include <slice/unity/api/scopes/internal/ice_middleware/QueryCtrl.h>
#include <slice/unity/api/scopes/internal/ice_middleware/Reply.h>
#include <unity/api/scopes/internal/ice_middleware/IceQueryCtrl.h>
#include <unity/api/scopes/internal/ice_middleware/IceReply.h>
#include <unity/api/scopes/internal/ice_middleware/RethrowException.h>
#include <unity/api/scopes/internal/QueryCtrlImpl.h>

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

IceScope::IceScope(IceMiddleware* mw_base, middleware::ScopePrx const& p) noexcept :
    MWObjectProxy(mw_base),
    IceObjectProxy(mw_base, p),
    MWScope(mw_base)
{
    // No assert here, it's legal to create a nil proxy for a scope (for example, when RegistryProxy::find()
    // doesn't find the scope).
}

IceScope::~IceScope() noexcept
{
}

QueryCtrlProxy IceScope::create_query(std::string const& q, MWReplyProxy const& reply)
{
    QueryCtrlProxy ctrl;
    try
    {
        auto self = middleware::ScopePrx::uncheckedCast(proxy());
        IceReplyProxy irp = dynamic_pointer_cast<IceReply>(reply);
        middleware::ReplyPrx rp = middleware::ReplyPrx::uncheckedCast(irp->proxy());
        middleware::QueryCtrlPrx qcp = self->createQuery(q, rp);

        IceQueryCtrlProxy iqc(new IceQueryCtrl(mw_base(), qcp));
        ctrl = QueryCtrlImpl::create(iqc, reply);
    }
    catch (Ice::Exception const& e)
    {
        rethrow_ice_ex(e);
    }
    return ctrl;
}

} // namespace ice_middleware

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

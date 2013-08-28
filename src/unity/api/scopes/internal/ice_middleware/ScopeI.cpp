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

//#include <unity/api/scopes/internal/ice_middleware/IceMiddleware.h>
//#include <unity/api/scopes/ScopeExceptions.h>
//#include <unity/UnityExceptions.h>

#include <unity/api/scopes/internal/ice_middleware/ScopeI.h>

#include <unity/api/scopes/internal/ice_middleware/slice/QueryCtrl.h>
#include <unity/api/scopes/internal/ice_middleware/slice/Reply.h>
#include <unity/api/scopes/internal/ice_middleware/IceQueryCtrl.h>
#include <unity/api/scopes/internal/ice_middleware/IceReply.h>

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

ScopeI::ScopeI(IceMiddleware* mw_base, ScopeObject::SPtr const& so) :
    mw_base_(mw_base),
    so_(so)
{
    assert(so);
}

ScopeI::~ScopeI() noexcept
{
}

// Methods below are marked with a
//    // noexcept
// comment. That's because they really are noexcept, but we can't say this in the signature
// because we are deriving from the Slice-generated base.

middleware::QueryCtrlPrx ScopeI::createQuery(string const& q,
                                             middleware::ReplyPrx const& r,
                                             Ice::Current const&) // noexcept
{
    middleware::QueryCtrlPrx ctrl;
    try
    {
        MWReplyProxy reply(new IceReply(mw_base_, r));
        MWQueryCtrlProxy mw_ctrl = so_->create_query(q, reply, mw_base_);
        assert(mw_ctrl);
        IceQueryCtrlProxy iqcp = dynamic_pointer_cast<IceQueryCtrl>(mw_ctrl);
        assert(iqcp);
        ctrl = middleware::QueryCtrlPrx::uncheckedCast(iqcp->proxy());
        assert(ctrl);
    }
    catch (...)
    {
        // TODO: log error
        throw;
    }
    return ctrl;
}

} // namespace ice_middleware

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

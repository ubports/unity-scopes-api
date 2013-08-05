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

#include <slice/unity/api/scopes/internal/ice_middleware/Reply.h>
#include <unity/api/scopes/internal/ice_middleware/IceReplyProxy.h>
#include <unity/api/scopes/internal/ReplyProxyImpl.h>

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

ScopeI::ScopeI(MiddlewareBase* mw_base, ScopeObject::SPtr const& so) :
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

// Process incoming query request from a client and forward it to the scope implementation.

void ScopeI::query(string const& q, middleware::ReplyPrx const& r, Ice::Current const&) // noexcept
{
    try
    {
        ReplyProxy::SPtr reply = ReplyProxyImpl::create(MWReplyProxy::SPtr(new IceReplyProxy(mw_base_, r)));
        so_->query(q, reply);
    }
    catch (...)
    {
    }
}

} // namespace ice_middleware

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

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

#include <unity/api/scopes/internal/ice_middleware/IceQueryCtrl.h>

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

IceQueryCtrl::IceQueryCtrl(IceMiddleware* mw_base, middleware::QueryCtrlPrx const& p) noexcept :
    MWObjectProxy(mw_base),
    IceObjectProxy(mw_base, p->ice_oneway()),
    MWQueryCtrl(mw_base)
{
    assert(p);
}

IceQueryCtrl::~IceQueryCtrl() noexcept
{
}

void IceQueryCtrl::cancel()
{
    try
    {
        auto self = middleware::QueryCtrlPrx::uncheckedCast(proxy()->ice_oneway());
        self->cancel();
    }
    catch (Ice::Exception const& e)
    {
        rethrow_ice_ex(e);
    }
}

void IceQueryCtrl::destroy()
{
    try
    {
        auto self = middleware::QueryCtrlPrx::uncheckedCast(proxy()->ice_oneway());
        self->destroy();
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

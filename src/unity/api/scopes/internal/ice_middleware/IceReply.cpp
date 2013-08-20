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

IceReply::IceReply(IceMiddleware* mw_base, middleware::ReplyPrx const& p) noexcept :
    MWObject(mw_base),
    IceObject(mw_base, p->ice_oneway()),
    MWReply(mw_base)
{
    assert(p);
}

IceReply::~IceReply() noexcept
{
}

void IceReply::push(string const& result)
{
    try
    {
        auto self = middleware::ReplyPrx::uncheckedCast(proxy());
        self->push(result);
    }
    catch (Ice::Exception const& e)
    {
        rethrow_ice_ex(e);
    }
}

void IceReply::finished()
{
    try
    {
        auto self = middleware::ReplyPrx::uncheckedCast(proxy());
        self->finished();
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

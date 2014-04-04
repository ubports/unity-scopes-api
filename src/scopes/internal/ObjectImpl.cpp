/*
 * Copyright (C) 2013 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Michi Henning <michi.henning@canonical.com>
 */

#include <unity/scopes/internal/ObjectImpl.h>

#include <unity/scopes/internal/MWObjectProxy.h>

using namespace std;

namespace unity
{

namespace scopes
{

namespace internal
{

ObjectImpl::ObjectImpl(MWProxy const& mw_proxy) :
    mw_proxy_(mw_proxy)
{
}

ObjectImpl::~ObjectImpl()
{
}

string ObjectImpl::identity()
{
    return mw_proxy_->identity();
}

string ObjectImpl::category()
{
    return mw_proxy_->category();
}

string ObjectImpl::endpoint()
{
    return mw_proxy_->endpoint();
}

int64_t ObjectImpl::timeout()
{
    return mw_proxy_->timeout();
}

string ObjectImpl::to_string()
{
    return mw_proxy_->to_string();
}

void ObjectImpl::ping()
{
    mw_proxy_->ping();
}

MWProxy ObjectImpl::proxy() const
{
    return mw_proxy_;
}

} // namespace internal

} // namespace scopes

} // namespace unity

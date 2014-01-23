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

#include <unity/scopes/internal/ObjectProxyImpl.h>

#include <unity/scopes/internal/MWObjectProxy.h>
#include <unity/scopes/ObjectProxy.h>

using namespace std;

namespace unity
{

namespace scopes
{

namespace internal
{

ObjectProxyImpl::ObjectProxyImpl(MWProxy const& mw_proxy) :
    mw_proxy_(mw_proxy)
{
}

ObjectProxyImpl::~ObjectProxyImpl()
{
}

Proxy ObjectProxyImpl::create(MWProxy const&)
{
    return make_shared<ObjectProxy>();
}

string ObjectProxyImpl::identity() const
{
    return mw_proxy_->identity();
}

string ObjectProxyImpl::endpoint() const
{
    return mw_proxy_->endpoint();
}

void ObjectProxyImpl::ping()
{
    mw_proxy_->ping();
}

MWProxy ObjectProxyImpl::proxy() const
{
    return mw_proxy_;
}

} // namespace internal

} // namespace scopes

} // namespace unity

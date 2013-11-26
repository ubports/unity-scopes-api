/*
 * Copyright (C) 2013 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the Lesser GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * Lesser GNU General Public License for more details.
 *
 * You should have received a copy of the Lesser GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Michi Henning <michi.henning@canonical.com>
 */

#include <scopes/ObjectProxy.h>

#include <scopes/internal/ObjectProxyImpl.h>

using namespace std;

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

class ObjectProxyImpl;

} // namespace internal

//! @cond

ObjectProxy::ObjectProxy()
{
}

ObjectProxy::ObjectProxy(internal::ObjectProxyImpl* pimpl) :
    p(pimpl)
{
}

ObjectProxy::~ObjectProxy() noexcept
{
}

string ObjectProxy::identity() const
{
    return p->identity();
}

string ObjectProxy::endpoint() const
{
    return p->endpoint();
}

internal::ObjectProxyImpl* ObjectProxy::pimpl() const noexcept
{
    return p.get();
}

//! @endcond

} // namespace scopes

} // namespace api

} // namespace unity

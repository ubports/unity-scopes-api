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

#include <unity/api/scopes/ScopeProxy.h>

#include <unity/api/scopes/internal/ScopeProxyImpl.h>

namespace unity
{

namespace api
{

namespace scopes
{

//! @cond

ScopeProxy::ScopeProxy(internal::ScopeProxyImpl* impl) :
    p(impl)
{
}

ScopeProxy::~ScopeProxy() noexcept
{
}

void ScopeProxy::query(std::string const& q, ReplyBase::SPtr const& reply)
{
    p->query(q, reply);
}

//! @endcond

} // namespace scopes

} // namespace api

} // namespace unity

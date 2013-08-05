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

#ifndef UNITY_INTERNAL_SCOPEPROXYIMPL_H
#define UNITY_INTERNAL_SCOPEPROXYIMPL_H

#include <unity/api/scopes/internal/MWScopeProxy.h>
#include <unity/api/scopes/ScopeProxy.h>

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

class RuntimeImpl;

class ScopeProxyImpl final
{
public:
    ScopeProxyImpl(MWScopeProxy::SPtr const& mw_proxy);
    ~ScopeProxyImpl() noexcept;

    void query(std::string const& q, ReplyBase::SPtr const& reply) const;

    static ScopeProxy::SPtr create(MWScopeProxy::SPtr const& mw_proxy);

private:
    MWScopeProxy::SPtr mw_proxy_;
};

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

#endif

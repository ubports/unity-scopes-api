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

#ifndef UNITY_INTERNAL_SCOPEIMPL_H
#define UNITY_INTERNAL_SCOPEIMPL_H

#include <unity/api/scopes/internal/MWScopeProxyFwd.h>
#include <unity/api/scopes/QueryCtrlProxyFwd.h>
#include <unity/api/scopes/ReplyBase.h>
#include <unity/api/scopes/ScopeProxyFwd.h>

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

class RuntimeImpl;

class ScopeImpl final
{
public:
    ScopeImpl(MWScopeProxy const& mw_proxy, RuntimeImpl* runtime);
    ~ScopeImpl() noexcept;

    QueryCtrlProxy create_query(std::string const& q, ReplyBase::SPtr const& reply) const;

    static ScopeProxy create(MWScopeProxy const& mw_proxy, RuntimeImpl* runtime);

private:
    MWScopeProxy mw_proxy_;
    RuntimeImpl* const runtime_;
};

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

#endif

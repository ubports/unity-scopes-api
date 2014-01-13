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

#ifndef UNITY_INTERNAL_SCOPEIMPL_H
#define UNITY_INTERNAL_SCOPEIMPL_H

#include <scopes/internal/MWScopeProxyFwd.h>
#include <scopes/internal/ObjectProxyImpl.h>
#include <scopes/QueryCtrlProxyFwd.h>
#include <scopes/ListenerBase.h>
#include <scopes/ActivationListener.h>
#include <scopes/ScopeProxyFwd.h>
#include <scopes/Result.h>
#include <scopes/Variant.h>
#include <string>

namespace unity
{

namespace api
{

namespace scopes
{

class Result;

namespace internal
{

class RuntimeImpl;

class ScopeImpl : public virtual ObjectProxyImpl
{
public:
    ScopeImpl(MWScopeProxy const& mw_proxy, RuntimeImpl* runtime, std::string const& scope_name);
    virtual ~ScopeImpl() noexcept;

    QueryCtrlProxy create_query(std::string const& q, VariantMap const& hints, SearchListener::SPtr const& reply) const;
    QueryCtrlProxy activate(Result const& result, VariantMap const& hints, ActivationListener::SPtr const& reply) const;
    QueryCtrlProxy preview(Result const& result, VariantMap const& hints, PreviewListener::SPtr const& reply) const;

    static ScopeProxy create(MWScopeProxy const& mw_proxy, RuntimeImpl* runtime, std::string const& scope_name);

private:
    MWScopeProxy fwd() const;

    RuntimeImpl* const runtime_;
    std::string scope_name_;
};

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

#endif

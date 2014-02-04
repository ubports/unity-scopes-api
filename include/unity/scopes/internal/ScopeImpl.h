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

#ifndef UNITY_SCOPES_INTERNAL_SCOPEIMPL_H
#define UNITY_SCOPES_INTERNAL_SCOPEIMPL_H

#include <unity/scopes/internal/MWScopeProxyFwd.h>
#include <unity/scopes/internal/ObjectProxyImpl.h>
#include <unity/scopes/QueryCtrlProxyFwd.h>
#include <unity/scopes/SearchListener.h>
#include <unity/scopes/ActivationListener.h>
#include <unity/scopes/PreviewListener.h>
#include <unity/scopes/Result.h>
#include <unity/scopes/ScopeProxyFwd.h>
#include <unity/scopes/Variant.h>
#include <string>

namespace unity
{

namespace scopes
{

class Result;
class FilterState;

namespace internal
{

class RuntimeImpl;

class ScopeImpl : public virtual ObjectProxyImpl
{
public:
    ScopeImpl(MWScopeProxy const& mw_proxy, RuntimeImpl* runtime, std::string const& scope_name);
    virtual ~ScopeImpl();

    QueryCtrlProxy create_query(std::string const& q, VariantMap const& hints, SearchListener::SPtr const& reply) const;
    QueryCtrlProxy create_query(std::string const& query_string, FilterState const& filter_state, VariantMap const& hints, SearchListener::SPtr const& reply) const;
    QueryCtrlProxy create_query(std::string const& query_string, std::string const& department_id, FilterState const& filter_state, VariantMap const& hints, SearchListener::SPtr const& reply) const;
    QueryCtrlProxy create_query(Query const& query, VariantMap const& hints, SearchListener::SPtr const& reply) const;
    QueryCtrlProxy activate(Result const& result, VariantMap const& hints, ActivationListener::SPtr const& reply) const;
    QueryCtrlProxy activate_preview_action(Result const& result, VariantMap const& hints, std::string const& action_id, ActivationListener::SPtr const& reply) const;
    QueryCtrlProxy preview(Result const& result, VariantMap const& hints, PreviewListener::SPtr const& reply) const;

    static ScopeProxy create(MWScopeProxy const& mw_proxy, RuntimeImpl* runtime, std::string const& scope_name);

private:
    MWScopeProxy fwd() const;

    RuntimeImpl* const runtime_;
    std::string scope_name_;
};

} // namespace internal

} // namespace scopes

} // namespace unity

#endif

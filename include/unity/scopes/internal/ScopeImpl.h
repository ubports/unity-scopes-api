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

#pragma once

#include <unity/scopes/ActivationListenerBase.h>
#include <unity/scopes/internal/MWScopeProxyFwd.h>
#include <unity/scopes/internal/ObjectImpl.h>
#include <unity/scopes/internal/SearchQueryBaseImpl.h>
#include <unity/scopes/PreviewListenerBase.h>
#include <unity/scopes/QueryCtrlProxyFwd.h>
#include <unity/scopes/Scope.h>
#include <unity/scopes/SearchListenerBase.h>

namespace unity
{

namespace scopes
{

class Result;
class FilterState;
class ActionMetadata;
class SearchMetadata;

namespace internal
{

class RuntimeImpl;

class ScopeImpl : public virtual unity::scopes::Scope, public virtual ObjectImpl
{
public:
    ScopeImpl(MWScopeProxy const& mw_proxy, std::string const& scope_id);
    virtual ~ScopeImpl();

    RuntimeImpl* runtime() const;

    virtual QueryCtrlProxy search(std::string const& q,
                                  SearchMetadata const& metadata,
                                  SearchListenerBase::SPtr const& reply) override;

    virtual QueryCtrlProxy search(std::string const& query_string,
                                  FilterState const& filter_state,
                                  SearchMetadata const& metadata,
                                  SearchListenerBase::SPtr const& reply) override;

    virtual QueryCtrlProxy search(std::string const& query_string,
                                  std::string const& department_id,
                                  FilterState const& filter_state,
                                  SearchMetadata const& metadata,
                                  SearchListenerBase::SPtr const& reply) override;

    QueryCtrlProxy search(std::string const& query_string,
                          std::string const& department_id,
                          FilterState const& filter_state,
                          SearchMetadata const& metadata,
                          SearchQueryBaseImpl::History const& history,
                          SearchListenerBase::SPtr const& reply);       // Not remote, hence not override

    QueryCtrlProxy search(CannedQuery const& query,
                          SearchMetadata const& metadata,
                          SearchQueryBaseImpl::History const& history,
                          SearchListenerBase::SPtr const& reply);       // Not remote, hence not override

    virtual QueryCtrlProxy activate(Result const& result,
                                    ActionMetadata const& metadata,
                                    ActivationListenerBase::SPtr const& reply) override;

    virtual QueryCtrlProxy perform_action(Result const& result,
                                          ActionMetadata const& hints,
                                          std::string const& widget_id,
                                          std::string const& action_id,
                                          ActivationListenerBase::SPtr const& reply) override;

    virtual QueryCtrlProxy preview(Result const& result,
                                   ActionMetadata const& hints,
                                   PreviewListenerBase::SPtr const& reply) override;

    virtual ChildScopeList child_scopes_ordered() override;
    virtual bool set_child_scopes_ordered(ChildScopeList const& child_scopes_ordered) override;

    static ScopeProxy create(MWScopeProxy const& mw_proxy, std::string const& scope_id);

private:
    MWScopeProxy fwd();

    RuntimeImpl* const runtime_;
    std::string scope_id_;
};

} // namespace internal

} // namespace scopes

} // namespace unity

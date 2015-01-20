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

#include <unity/scopes/internal/ScopeObjectBase.h>
#include <unity/scopes/internal/QueryObjectBase.h>
#include <unity/scopes/QueryBase.h>

#include <functional>
#include <string>

namespace unity
{

namespace scopes
{

class ScopeBase;
class ActionMetadata;
class SearchMetadata;
class Result;

namespace internal
{

class MiddlewareBase;

// A ScopeObject sits in between the incoming requests from the middleware layer and the
// ScopeBase-derived implementation provided by the scope. It forwards incoming
// queries to the actual scope. This allows us to intercept all queries for a scope.

class ScopeObject final : public ScopeObjectBase
{
public:
    UNITY_DEFINES_PTRS(ScopeObject);

    ScopeObject(ScopeBase* scope_base, bool debug_mode = false);
    virtual ~ScopeObject();

    // Remote operation implementations
    virtual MWQueryCtrlProxy search(CannedQuery const& q,
                                          SearchMetadata const& hints,
                                          MWReplyProxy const& reply,
                                          InvokeInfo const& info) override;

    virtual MWQueryCtrlProxy activate(Result const& result,
                              ActionMetadata const& hints,
                              MWReplyProxy const &reply,
                              InvokeInfo const& info) override;

    virtual MWQueryCtrlProxy perform_action(Result const& result,
                                            ActionMetadata const& hints,
                                            std::string const& widget_id,
                                            std::string const& action_id,
                                            MWReplyProxy const &reply,
                                            InvokeInfo const& info) override;

    virtual MWQueryCtrlProxy preview(Result const& result,
                                     ActionMetadata const& hints,
                                     MWReplyProxy const& reply,
                                     InvokeInfo const& info) override;

    virtual bool debug_mode() const override;

private:
    MWQueryCtrlProxy query(MWReplyProxy const& reply, MiddlewareBase* mw_base,
        std::string const& method,
        std::function<QueryBase::SPtr(void)> const& query_factory_fun,
        std::function<QueryObjectBase::SPtr(QueryBase::SPtr, MWQueryCtrlProxy)> const& query_object_factory_fun);
    ScopeBase* const scope_base_;
    bool const debug_mode_;
};

} // namespace internal

} // namespace scopes

} // namespace unity

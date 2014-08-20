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
 * Authored by: Marcus Tomlinson <marcus.tomlinson@canonical.com>
 */

#ifndef UNITY_SCOPES_INTERNAL_SMARTSCOPES_SSSCOPEOBJECT_H
#define UNITY_SCOPES_INTERNAL_SMARTSCOPES_SSSCOPEOBJECT_H

#include <unity/scopes/internal/ScopeObjectBase.h>
#include <unity/scopes/internal/smartscopes/SSQueryCtrlObject.h>
#include <unity/scopes/internal/smartscopes/SSQueryObject.h>
#include <unity/scopes/internal/smartscopes/SSRegistryObject.h>
#include <unity/scopes/QueryBase.h>

#include <string>

namespace unity
{

namespace scopes
{

class ActionMetadata;

namespace internal
{

namespace smartscopes
{

class SmartScope;

class SSScopeObject final : public ScopeObjectBase
{
public:
    UNITY_DEFINES_PTRS(SSScopeObject);

    SSScopeObject(std::string const& ss_scope_id, MiddlewareBase::SPtr middleware, SSRegistryObject::SPtr ss_registry);
    virtual ~SSScopeObject();

    // Remote operation implementations
    MWQueryCtrlProxy search(CannedQuery const& q,
                                  SearchMetadata const& hints,
                                  MWReplyProxy const& reply,
                                  InvokeInfo const& info) override;

    MWQueryCtrlProxy activate(Result const& result,
                              ActionMetadata const& hints,
                              MWReplyProxy const& reply,
                              InvokeInfo const& info) override;

    MWQueryCtrlProxy perform_action(Result const& result,
                                    ActionMetadata const& hints,
                                    std::string const& widget_id,
                                    std::string const& action_id,
                                    MWReplyProxy const& reply,
                                    InvokeInfo const& info) override;

    MWQueryCtrlProxy preview(Result const& result,
                             ActionMetadata const& hints,
                             MWReplyProxy const& reply,
                             InvokeInfo const& info) override;

    bool debug_mode() const override;

private:
    MWQueryCtrlProxy query(InvokeInfo const& info,
                           MWReplyProxy const& reply,
                           std::function<QueryBase::SPtr(void)> const& query_factory_fun,
                           std::function<void(QueryBase::SPtr)> const& query_object_fun);

private:
    std::string ss_scope_id_;

    SSQueryCtrlObject::SPtr co_;
    SSQueryObject::SPtr qo_;

    std::unique_ptr<SmartScope> const smartscope_;
    SSRegistryObject::SPtr ss_registry_;
};

}  // namespace smartscopes

}  // namespace internal

}  // namespace scopes

}  // namespace unity

#endif  // UNITY_SCOPES_INTERNAL_SMARTSCOPES_SSSCOPEOBJECT_H

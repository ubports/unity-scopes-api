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
#include <unity/scopes/internal/smartscopes/SSRegistryObject.h>
#include <unity/scopes/QueryBase.h>

#include <string>

namespace unity
{

namespace scopes
{

namespace internal
{

namespace smartscopes
{

class SmartScope;

class SSScopeObject final : public ScopeObjectBase
{
public:
  UNITY_DEFINES_PTRS(SSScopeObject);

  SSScopeObject(RuntimeImpl* runtime, SSRegistryObject::SPtr registry);
  virtual ~SSScopeObject() noexcept;

  // Remote operation implementations
  MWQueryCtrlProxy create_query(std::string const& q,
                                VariantMap const& hints,
                                MWReplyProxy const& reply,
                                InvokeInfo const& info) override;

  MWQueryCtrlProxy activate(Result const& result,
                            VariantMap const& hints,
                            MWReplyProxy const &reply,
                            InvokeInfo const& info) override;

  MWQueryCtrlProxy activate_preview_action(Result const& result,
                                           VariantMap const& hints,
                                           std::string const& action_id,
                                           MWReplyProxy const &reply,
                                           InvokeInfo const& info) override;

  MWQueryCtrlProxy preview(Result const& result,
                           VariantMap const& hints,
                           MWReplyProxy const& reply,
                           InvokeInfo const& info) override;

private:
  MWQueryCtrlProxy query(MWReplyProxy const& reply, MiddlewareBase* mw_base,
      std::function<QueryBase::SPtr(void)> const& query_factory_fun,
      std::function<QueryObjectBase::SPtr(QueryBase::SPtr)> const& query_object_factory_fun);

private:
  RuntimeImpl* const runtime_;
  SSRegistryObject::SPtr registry_;
  std::unique_ptr<SmartScope> const smartscope_;
};

} // namespace smartscopes

} // namespace internal

} // namespace scopes

} // namespace unity

#endif // UNITY_SCOPES_INTERNAL_SMARTSCOPES_SSSCOPEOBJECT_H

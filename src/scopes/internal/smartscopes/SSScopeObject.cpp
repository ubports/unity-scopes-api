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

#include <unity/scopes/internal/smartscopes/SSScopeObject.h>
#include <unity/scopes/internal/smartscopes/SmartScope.h>
#include <unity/scopes/internal/smartscopes/SSQueryObject.h>

#include <cassert>

namespace unity
{

namespace scopes
{

namespace internal
{

namespace smartscopes
{

SSScopeObject::SSScopeObject(RuntimeImpl* runtime, SSRegistryObject::SPtr registry) :
  runtime_(runtime),
  registry_(registry),
  smartscope_(new SmartScope())
{
  assert(runtime);
}

SSScopeObject::~SSScopeObject() noexcept
{
}

MWQueryCtrlProxy SSScopeObject::create_query(std::string const& q,
                                             VariantMap const& hints,
                                             MWReplyProxy const& reply,
                                             InvokeInfo const& info)
{
  return query(reply, info.mw,
          [&q, &hints, &info, this]() -> QueryBase::SPtr {
              return this->smartscope_->create_query(info.id, q, hints);
          },
          [&reply](QueryBase::SPtr query_base, MWQueryCtrlProxy ctrl_proxy) -> QueryObjectBase::SPtr {
              return std::make_shared<SSQueryObject>(query_base, reply, ctrl_proxy);
          }
  );
}

MWQueryCtrlProxy SSScopeObject::activate(Result const& result,
                                         VariantMap const& hints,
                                         MWReplyProxy const& reply,
                                         InvokeInfo const& info)
{
  (void)result;
  (void)hints;
  (void)reply;
  (void)info;
  return MWQueryCtrlProxy();
}

MWQueryCtrlProxy SSScopeObject::activate_preview_action(Result const& result,
                                                        VariantMap const& hints,
                                                        std::string const& action_id,
                                                        MWReplyProxy const &reply,
                                                        InvokeInfo const& info)
{
  (void)result;
  (void)hints;
  (void)action_id;
  (void)reply;
  (void)info;
  return MWQueryCtrlProxy();
}

MWQueryCtrlProxy SSScopeObject::preview(Result const& result,
                                        VariantMap const& hints,
                                        MWReplyProxy const& reply,
                                        InvokeInfo const& info)
{
  (void)result;
  (void)hints;
  (void)reply;
  (void)info;
  return MWQueryCtrlProxy();
}

MWQueryCtrlProxy SSScopeObject::query(MWReplyProxy const& reply, MiddlewareBase* mw_base,
        std::function<QueryBase::SPtr()> const& query_factory_fun,
        std::function<QueryObjectBase::SPtr(QueryBase::SPtr, MWQueryCtrlProxy)> const& query_object_factory_fun)
{
}

} // namespace smartscopes

} // namespace internal

} // namespace scopes

} // namespace unity

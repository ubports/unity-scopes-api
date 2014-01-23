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

#include <cassert>

namespace unity
{

namespace scopes
{

namespace internal
{

namespace smartscopes
{

SSScopeObject::SSScopeObject(RuntimeImpl* runtime) :
  runtime_(runtime)
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
  (void)q;
  (void)hints;
  (void)reply;
  (void)info;
  return MWQueryCtrlProxy();
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

} // namespace smartscopes

} // namespace internal

} // namespace scopes

} // namespace unity

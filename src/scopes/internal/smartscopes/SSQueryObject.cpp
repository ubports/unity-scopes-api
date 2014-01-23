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

#include <unity/scopes/internal/smartscopes/SSQueryObject.h>

#include <cassert>

using namespace std;
using namespace unity::scopes::internal;

namespace unity
{

namespace scopes
{

namespace internal
{

namespace smartscopes
{

SSQueryObject::SSQueryObject(shared_ptr<QueryBase> const& query_base,
                         MWReplyProxy const& reply,
                         MWQueryCtrlProxy const& ctrl) :
    QueryObjectBase(),
    query_base_(query_base),
    reply_(reply),
    ctrl_(ctrl),
    pushable_(true)
{
    assert(query_base);
    assert(reply);
    assert(ctrl);
}

SSQueryObject::~SSQueryObject() noexcept
{
}

void SSQueryObject::run(MWReplyProxy const& reply) noexcept
{
  (void)reply;
}

void SSQueryObject::cancel()
{
}

bool SSQueryObject::pushable() const noexcept
{
  return pushable_;
}

void SSQueryObject::set_self(QueryObjectBase::SPtr const& self) noexcept
{
  (void)self;
}

} // namespace smartscopes

} // namespace internal

} // namespace scopes

} // namespace unity

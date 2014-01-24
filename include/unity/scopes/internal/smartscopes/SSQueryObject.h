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

#ifndef UNITY_SCOPES_INTERNAL_SMARTSCOPES_SSQUERYOBJECT_H
#define UNITY_SCOPES_INTERNAL_SMARTSCOPES_SSQUERYOBJECT_H

#include <unity/scopes/internal/QueryObjectBase.h>
#include <unity/scopes/internal/MWReplyProxyFwd.h>
#include <unity/scopes/internal/MWQueryCtrlProxyFwd.h>
#include <unity/scopes/ReplyProxyFwd.h>

#include <atomic>

namespace unity
{

namespace scopes
{

class QueryBase;

namespace internal
{

namespace smartscopes
{

class SSQueryObject : public QueryObjectBase
{
public:
    UNITY_DEFINES_PTRS(SSQueryObject);

    SSQueryObject(std::shared_ptr<QueryBase> const& query_base, MWReplyProxy const& reply);
    virtual ~SSQueryObject() noexcept;

    // Remote operation implementations
    virtual void run(MWReplyProxy const& reply, InvokeInfo const& info) noexcept override;

    // Local methods
    virtual void cancel(InvokeInfo const& info) override;                   // Called locally, by QueryCtrlObject.
    virtual bool pushable(InvokeInfo const& info) const noexcept override;  // Called locally, by ReplyImpl

    // Called by create_query(), to hold the reference count high until the run call arrives via the middleware,
    // and we can pass the shared_ptr to the ReplyImpl.
    void set_self(QueryObjectBase::SPtr const& self) noexcept override;

protected:
    std::shared_ptr<QueryBase> query_base_;
    MWReplyProxy reply_;
    std::weak_ptr<ReplyBase> reply_proxy_;
    std::atomic_bool pushable_;
    QueryObjectBase::SPtr self_;
};

} // namespace smartscopes

} // namespace internal

} // namespace scopes

} // namespace unity

#endif // UNITY_SCOPES_INTERNAL_SMARTSCOPES_SSQUERYOBJECT_H

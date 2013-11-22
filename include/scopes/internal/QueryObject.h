/*
 * Copyright (C) 2013 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Michi Henning <michi.henning@canonical.com>
 */

#ifndef UNITY_API_REPLYS_INTERNAL_QUERYOBJECT_H
#define UNITY_API_REPLYS_INTERNAL_QUERYOBJECT_H

#include <scopes/internal/AbstractObject.h>
#include <scopes/internal/MWReplyProxyFwd.h>
#include <scopes/internal/MWQueryCtrlProxyFwd.h>
#include <scopes/ReplyProxyFwd.h>

#include <atomic>
#include <mutex>

namespace unity
{

namespace api
{

namespace scopes
{

class QueryBase;

namespace internal
{

class QueryCtrlObject;

// A QueryObject sits in between the incoming requests from the middleware layer and the
// QueryBase-derived implementation. This allows us to receive cancel requests. In turn,
// the implementation of this object ensures that the corresponding ReplyObject is disabled.
// TODO: Probably need to flesh out this comment.

class QueryObject final : public AbstractObject
{
public:
    UNITY_DEFINES_PTRS(QueryObject);

    QueryObject(std::shared_ptr<QueryBase> const& query_base, MWReplyProxy const& reply, MWQueryCtrlProxy const& ctrl);
    virtual ~QueryObject() noexcept;

    // Remote operation implementation
    void run(MWReplyProxy const& reply) noexcept;

    // Called locally only, by QueryCtrlObject.
    void cancel();

    bool pushable() const noexcept; // Called locallly only, by ReplyImpl

    // Called by create_query(), to hold the reference count high until the run call arrives via the middleware,
    // and we can pass the shared_ptr to the ReplyImpl.
    void set_self(SPtr const& self);

private:
    std::shared_ptr<QueryBase> query_base_;
    MWReplyProxy reply_;
    std::weak_ptr<Reply> reply_proxy_;
    MWQueryCtrlProxy const ctrl_;
    std::atomic_bool pushable_;
    SPtr self_;
};

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

#endif

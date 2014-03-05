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
#include <unity/scopes/ReplyProxyFwd.h>

#include <condition_variable>
#include <map>
#include <mutex>

namespace unity
{

namespace scopes
{

class QueryBase;

namespace internal
{

namespace smartscopes
{

struct SSQuery
{
    UNITY_DEFINES_PTRS(SSQuery);

    enum QueryType
    {
        Query, Activation, Preview
    };

    SSQuery(QueryType type, std::shared_ptr<QueryBase> base, int cardinality, MWReplyProxy reply)
        : q_type(type),
          q_base(base),
          q_cardinality(cardinality),
          q_reply(reply) {}

    QueryType q_type;
    std::shared_ptr<QueryBase> q_base;
    int q_cardinality;
    MWReplyProxy q_reply;
    std::weak_ptr<unity::scopes::Reply> q_reply_proxy;
    bool q_pushable = true;
};

class SSQueryObject : public QueryObjectBase, public std::enable_shared_from_this<SSQueryObject>
{
public:
    UNITY_DEFINES_PTRS(SSQueryObject);

    SSQueryObject();
    virtual ~SSQueryObject() noexcept;

    // Remote operation implementations
    virtual void run(MWReplyProxy const& reply, InvokeInfo const& info) noexcept override;

    // Local methods
    virtual void cancel(InvokeInfo const& info) override;                     // Called locally, by QueryCtrlObject.
    virtual bool pushable(InvokeInfo const& info) const noexcept override;    // Called locally, by ReplyImpl
    virtual int cardinality(InvokeInfo const& info) const noexcept override;  // Called locally, by ReplyImpl

    void set_self(QueryObjectBase::SPtr const& self) noexcept override;

    void add_query(SSQuery::QueryType query_type,
                   std::shared_ptr<QueryBase> const& query_base,
                   int cardinality,
                   MWReplyProxy const& reply);
    void add_query(SSQuery::QueryType query_type,
                   std::shared_ptr<QueryBase> const& query_base,
                   MWReplyProxy const& reply);

private:
    void run_query(SSQuery::SPtr query, MWReplyProxy const& reply);
    void run_preview(SSQuery::SPtr query, MWReplyProxy const& reply);
    void run_activation(SSQuery::SPtr query, MWReplyProxy const& reply);

private:
    mutable std::mutex queries_mutex_;

    std::map<std::string, SSQuery::SPtr> queries_;  // reply ID : query
};

}  // namespace smartscopes

}  // namespace internal

}  // namespace scopes

}  // namespace unity

#endif  // UNITY_SCOPES_INTERNAL_SMARTSCOPES_SSQUERYOBJECT_H

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

#include <unity/scopes/internal/MWReply.h>
#include <unity/scopes/internal/ReplyImpl.h>
#include <unity/scopes/ScopeExceptions.h>
#include <unity/scopes/SearchReply.h>
#include <unity/scopes/SearchQuery.h>

#include <iostream>
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

SSQueryObject::SSQueryObject()
    : QueryObjectBase()
{
}

SSQueryObject::~SSQueryObject() noexcept
{
}

void SSQueryObject::run(MWReplyProxy const& reply, InvokeInfo const& info) noexcept
{
    decltype(queries_.begin()) query;
    SearchReplyProxy q_reply_proxy;

    try
    {
        QueryBase::SPtr q_base;

        {
            std::lock_guard<std::mutex> lock(queries_mutex_);

            // find the targeted query according to InvokeInfo
            query = queries_.find(info.id);

            if (query == end(queries_))
            {
                throw ObjectNotExistException("Query does not exist", info.id);
            }

            q_base = query->second.q_base;

            // Create the reply proxy to pass to query_base_ and keep a weak_ptr, which we will need
            // if cancel() is called later.
            q_reply_proxy = ReplyImpl::create(reply, shared_from_this());
            assert(q_reply_proxy);
            query->second.q_reply_proxy = q_reply_proxy;
        }

        // Synchronous call into scope implementation.
        // On return, replies for the query may still be outstanding.
        auto search_query = dynamic_pointer_cast<SearchQuery>(q_base);
        assert(search_query);
        search_query->run(q_reply_proxy);
    }
    catch (std::exception const& e)
    {
        std::lock_guard<std::mutex> lock(queries_mutex_);

        query->second.q_pushable = false;
        // TODO: log error
        reply->finished(ListenerBase::Error, e.what());  // Oneway, can't block
        cerr << "SSQueryObject::run(): " << e.what() << endl;
    }
    catch (...)
    {
        std::lock_guard<std::mutex> lock(queries_mutex_);

        query->second.q_pushable = false;
        // TODO: log error
        reply->finished(ListenerBase::Error, "unknown exception");  // Oneway, can't block
        cerr << "SSQueryObject::run(): unknown exception" << endl;
    }

    {
        std::lock_guard<std::mutex> lock(queries_mutex_);

        // the query is complete so it is no longer needed
        queries_.erase(query);
        replies_.erase(reply->identity());
    }
}

void SSQueryObject::cancel(InvokeInfo const& info)
{
    std::lock_guard<std::mutex> lock(queries_mutex_);

    // find the targeted query according to InvokeInfo
    auto query = queries_.find(info.id);

    if (query == end(queries_))
    {
        throw ObjectNotExistException("Query does not exist", info.id);
    }

    QueryBase::SPtr const& q_base = query->second.q_base;
    MWReplyProxy const& q_reply = query->second.q_reply;

    // this query is cancelled so replies are no longer pushable
    query->second.q_pushable = false;
    auto rp = query->second.q_reply_proxy.lock();
    if (rp)
    {
        // Send finished() to up-stream client to tell him the query is done.
        // We send via the MWReplyProxy here because that allows passing
        // a ListenerBase::Reason (whereas the public ReplyProxy does not).
        q_reply->finished(ListenerBase::Cancelled, "");  // Oneway, can't block
    }

    // Forward the cancellation to the query base (which in turn will forward it to any subqueries).
    // The query base also calls the cancelled() callback to inform the application code.
    q_base->cancel();
}

bool SSQueryObject::pushable(InvokeInfo const& info) const noexcept
{
    std::lock_guard<std::mutex> lock(queries_mutex_);

    // find corresponding scope ID to the reply ID requested
    auto reply = replies_.find(info.id);
    if (reply == end(replies_))
    {
        return false;
    }

    std::string scope_id = reply->second;

    // find query in queries_ from scope ID
    auto query = queries_.find(scope_id);
    if (query == end(queries_))
    {
        return false;
    }

    return query->second.q_pushable;
}

void SSQueryObject::set_self(QueryObjectBase::SPtr const& /*self*/) noexcept
{
    ///! TODO: remove
}

void SSQueryObject::add_query(std::string const& scope_id, QueryBase::SPtr const& query_base, MWReplyProxy const& reply)
{
    std::lock_guard<std::mutex> lock(queries_mutex_);

    // add the new query struct to queries_
    queries_[scope_id] = SSQuery{query_base, reply, std::weak_ptr<ReplyBase>(), true};

    // ...as well as a mapping of reply ID to scope ID in replies_
    replies_[reply->identity()] = scope_id;
}

}  // namespace smartscopes

}  // namespace internal

}  // namespace scopes

}  // namespace unity

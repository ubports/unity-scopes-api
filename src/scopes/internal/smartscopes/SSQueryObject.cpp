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
#include <unity/scopes/PreviewReply.h>
#include <unity/scopes/PreviewQuery.h>

#include <iostream>
#include <cassert>
#include <thread>

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

void SSQueryObject::run(MWReplyProxy const& reply, InvokeInfo const& /*info*/) noexcept
{
    decltype(queries_.begin()) query_it;

    try
    {
        {
            std::lock_guard<std::mutex> lock(queries_mutex_);

            // find the targeted query according to InvokeInfo
            query_it = queries_.find(reply->identity());

            if (query_it == end(queries_))
            {
                throw ObjectNotExistException("Query does not exist", reply->identity());
            }
        }

        if (query_it->second->q_pushable)
        {
            if (query_it->second->q_type == SSQuery::Query)
            {
                run_query(query_it->second, reply);
            }
            else if (query_it->second->q_type == SSQuery::Preview)
            {
                run_preview(query_it->second, reply);
            }
            else if (query_it->second->q_type == SSQuery::Activation)
            {
                run_activation(query_it->second, reply);
            }
        }
    }
    catch (std::exception const& e)
    {
        std::lock_guard<std::mutex> lock(queries_mutex_);

        query_it->second->q_pushable = false;
        // TODO: log error
        reply->finished(ListenerBase::Error, e.what());  // Oneway, can't block
        cerr << "SSQueryObject::run(): " << e.what() << endl;
    }
    catch (...)
    {
        std::lock_guard<std::mutex> lock(queries_mutex_);

        query_it->second->q_pushable = false;
        // TODO: log error
        reply->finished(ListenerBase::Error, "unknown exception");  // Oneway, can't block
        cerr << "SSQueryObject::run(): unknown exception" << endl;
    }

    {
        std::lock_guard<std::mutex> lock(queries_mutex_);

        // the query is complete so this is no longer needed
        queries_.erase(reply->identity());
    }
}

void SSQueryObject::cancel(InvokeInfo const& info)
{
    std::lock_guard<std::mutex> lock(queries_mutex_);

    std::string reply_id = info.id;
    reply_id.resize(reply_id.size() - 2);  // remove the ".c" suffix

    // find the targeted query according to InvokeInfo
    auto query_it = queries_.find(reply_id);

    if (query_it == end(queries_))
    {
        throw ObjectNotExistException("Query does not exist", info.id);
    }

    QueryBase::SPtr const& q_base = query_it->second->q_base;
    MWReplyProxy const& q_reply = query_it->second->q_reply;

    // this query is cancelled so replies are no longer pushable
    query_it->second->q_pushable = false;
    auto rp = query_it->second->q_reply_proxy.lock();
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

    // find query in queries_ from reply ID
    auto query_it = queries_.find(info.id);
    assert(query_it != end(queries_));

    return query_it->second->q_pushable;
}

void SSQueryObject::set_self(QueryObjectBase::SPtr const& /*self*/) noexcept
{
    ///! TODO: remove
}

void SSQueryObject::add_query(SSQuery::QueryType query_type, QueryBase::SPtr const& query_base,
                              MWReplyProxy const& reply)
{
    std::unique_lock<std::mutex> lock(queries_mutex_);

    // add the new query struct to queries_
    queries_[reply->identity()] = std::make_shared<SSQuery>(query_type, query_base, reply);
}

void SSQueryObject::run_query(SSQuery::SPtr query, MWReplyProxy const& reply)
{
    QueryBase::SPtr q_base;
    SearchReplyProxy q_reply_proxy;
    SearchQuery::SPtr search_query;

    {
        std::lock_guard<std::mutex> lock(queries_mutex_);

        q_base = query->q_base;

        // Create the reply proxy and keep a weak_ptr, which we will need
        // if cancel() is called later.
        q_reply_proxy = ReplyImpl::create(reply, shared_from_this());
        assert(q_reply_proxy);
        query->q_reply_proxy = q_reply_proxy;

        // Synchronous call into scope implementation.
        // On return, replies for the query may still be outstanding.
        search_query = dynamic_pointer_cast<SearchQuery>(q_base);
        assert(search_query);
    }

    search_query->run(q_reply_proxy);
}

void SSQueryObject::run_preview(SSQuery::SPtr query, MWReplyProxy const& reply)
{
    QueryBase::SPtr q_base;
    PreviewReplyProxy q_reply_proxy;
    PreviewQuery::SPtr preview_query;

    {
        std::lock_guard<std::mutex> lock(queries_mutex_);

        q_base = query->q_base;

        // Create the reply proxy and keep a weak_ptr, which we will need
        // if cancel() is called later.
        q_reply_proxy = ReplyImpl::create_preview_reply(reply, shared_from_this());
        assert(q_reply_proxy);
        query->q_reply_proxy = q_reply_proxy;

        // Synchronous call into scope implementation.
        // On return, replies for the query may still be outstanding.
        preview_query = dynamic_pointer_cast<PreviewQuery>(q_base);
        assert(preview_query);
    }

    preview_query->run(q_reply_proxy);
}

void SSQueryObject::run_activation(SSQuery::SPtr query, MWReplyProxy const& reply)
{
    ///! TODO
    (void)query;
    (void)reply;
}

}  // namespace smartscopes

}  // namespace internal

}  // namespace scopes

}  // namespace unity

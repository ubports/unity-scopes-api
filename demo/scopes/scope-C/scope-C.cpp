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
 * Authored by: Michi Henning <michi.henning@canonical.com>
 */

#include <scopes/ScopeBase.h>
#include <scopes/Result.h>
#include <scopes/Category.h>
#include <scopes/Reply.h>

#include <algorithm>
#include <condition_variable>
#include <iostream>
#include <list>
#include <mutex>
#include <unistd.h>

#define EXPORT __attribute__ ((visibility ("default")))

using namespace std;
using namespace unity::api::scopes;

// Simple queue that stores query-string/reply pairs, using MyQuery* as a key for removal.
// The put() method adds a pair at the tail, and the get() method returns a pair at the head.
// get() suspends the caller until an item is available or until the queue is told to finish.
// get() returns true if it returns a pair, false if the queue was told to finish.
// remove() searches for the entry with the given key and erases it.

class MyQuery;

class Queue
{
public:
    void put(MyQuery const* query, string const& query_string, ReplyProxy const& reply_proxy)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        queries_.push_back(QueryData { query, query_string, reply_proxy });
        condvar_.notify_one();
    }

    bool get(string& query_string, ReplyProxy& reply_proxy)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        condvar_.wait(lock, [this] { return !queries_.empty() || done_; });
        if (done_)
        {
            condvar_.notify_all();
        }
        else
        {
            auto qd = queries_.front();
            queries_.pop_front();
            query_string = qd.query_string;
            reply_proxy = qd.reply_proxy;
        }
        return !done_;
    }

    void remove(MyQuery const* query)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        QueryData qd { query, "", nullptr };
        auto it = std::find(queries_.begin(), queries_.end(), qd);
        if (it != queries_.end())
        {
            cerr << "Queue: removed query: " << it->query_string << endl;
            queries_.erase(it);
        }
        else
        {
            cerr << "Queue: did not find entry to be removed" << endl;
        }
    }

    void finish()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        queries_.clear();
        done_ = true;
        condvar_.notify_all();
    }

    Queue()
        : done_(false)
    {
    }

private:
    struct QueryData
    {
        MyQuery const* query;
        string query_string;
        ReplyProxy reply_proxy;

        bool operator==(QueryData const& rhs) const
        {
            return query == rhs.query;
        }
    };

    std::list<QueryData> queries_;
    bool done_;
    std::mutex mutex_;
    std::condition_variable condvar_;
};

// Example scope C: Does not use the query's run() method other than to remember the query.
// The run() method of the scope acts as a worker thread to push replies to remembered queries.
// This example shows that letting run() return immediately is OK, and that the MyQuery instance stays
// alive as long as it can still be cancelled, which is while there is at least one
// ReplyProxy still in existence for this query.

class MyQuery : public QueryBase
{
public:
    MyQuery(string const& query, Queue& queue) :
        query_(query),
        queue_(queue)
    {
        cerr << "My Query created" << endl;
    }

    ~MyQuery() noexcept
    {
        cerr << "My Query destroyed" << endl;
    }

    virtual void cancelled() override
    {
        // Remove this query from the queue, if it is still there.
        // If it isn't, and the worker thread is still working on this
        // query, the worker thread's next call to push() will return false,
        // causing the worker thread to stop working on this query.
        queue_.remove(this);
        cerr << "scope-C: \"" + query_ + "\" cancelled" << endl;
    }

    virtual void run(ReplyProxy const& reply) override
    {
        queue_.put(this, query_, reply);
        cerr << "scope-C: run() returning" << endl;
    }

private:
    string query_;
    Queue& queue_;
};

class MyScope : public ScopeBase
{
public:
    virtual int start(string const& scope_name, RegistryProxy const&) override
    {
        scope_name_ = scope_name;
        return VERSION;
    }

    virtual void stop() override
    {
        queue.finish();
    }

    virtual void run() override
    {
        for (;;)
        {
            string query;
            ReplyProxy reply;
            if (!queue.get(query, reply))
            {
                cerr << "worker thread terminating, queue was cleared" << endl;
                break;  // stop() was called.
            }
            auto cat = reply->lookup_category("cat1");
            if (cat == nullptr)
            {
                cat = reply->register_category("cat1", "Category 1", "", "{}");
            }
            for (int i = 1; i < 4; ++i)
            {
                Result result(cat);
                result.set_uri("uri");
                result.set_title("scope-C: result " + to_string(i) + " for query \"" + query + "\"");
                result.set_art("icon");
                result.set_dnd_uri("dnd_uri");
                if (!reply->push(result))
                {
                    break; // Query was cancelled
                }
                sleep(1);
            }
        }
    }

    virtual QueryBase::UPtr create_query(string const& q, VariantMap const&) override
    {
        cout << scope_name_ << ": created query: \"" << q << "\"" << endl;
        return QueryBase::UPtr(new MyQuery(q, queue));
    }

private:
    string scope_name_;
    Queue queue;
};

extern "C"
{

    EXPORT
    unity::api::scopes::ScopeBase*
    // cppcheck-suppress unusedFunction
    UNITY_API_SCOPE_CREATE_FUNCTION()
    {
        return new MyScope;
    }

    EXPORT
    void
    // cppcheck-suppress unusedFunction
    UNITY_API_SCOPE_DESTROY_FUNCTION(unity::api::scopes::ScopeBase* scope_base)
    {
        delete scope_base;
    }

}

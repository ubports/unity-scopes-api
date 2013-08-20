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

#include <unity/api/scopes/RegistryProxyFwd.h>
#include <unity/api/scopes/Reply.h>
#include <unity/api/scopes/ScopeBase.h>

#include <atomic>
#include <condition_variable>
#include <iostream>
#include <list>
#include <mutex>

#include <unistd.h>

#define EXPORT __attribute__ ((visibility ("default")))

using namespace std;
using namespace unity::api::scopes;

// Simple queue that stores query/reply pairs.
// The put() method adds a pair at the tail, and the get() method returns a pair at the head.
// get() suspends the caller until an item is available or until the queue is told to finish.
// get() returns true if it returns a pair, false if the queue was told to finish.

class Queue
{
public:
    void put(string const& query, ReplyProxy const& reply_proxy)
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            queries_.push_back(QueryData { query, reply_proxy });
        }
        condvar_.notify_one();
    }

    bool get(string& query, ReplyProxy& reply_proxy)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        condvar_.wait(lock, [this] { return !queries_.empty() || done_; });
        if (done_)
        {
            lock.unlock();
            condvar_.notify_all();
        }
        else
        {
            auto qd = queries_.front();
            queries_.pop_front();
            query = qd.query;
            reply_proxy = qd.reply_proxy;
        }
        return !done_;
    }

    void finish()
    {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            done_ = true;
        }
        condvar_.notify_all();
    }

    Queue()
        : done_(false)
    {
    }

private:
    struct QueryData
    {
        string query;
        ReplyProxy reply_proxy;
    };

    std::list<QueryData> queries_;
    bool done_;
    std::mutex mutex_;
    std::condition_variable condvar_;
};

class MyQuery : public QueryBase
{
public:
    MyQuery(string const& scope_name,
            string const& query,
            std::function<void(string const&, ReplyProxy const&)> const& add_func) :
        QueryBase(scope_name),
        scope_name_(scope_name),
        query_(query),
        add_func_(add_func)
    {
        cerr << "query instance for \"" << scope_name_ << ":" << query << "\" created" << endl;
    }

    virtual ~MyQuery() noexcept
    {
        cerr << "query instance for \"" << scope_name_ << ":" << query_ << "\" destroyed" << endl;
    }

    virtual void cancelled(ReplyProxy const&) override
    {
        // Informational callback to let a query know when it was cancelled. The query should
        // clean up any resources it has allocated, stop pushing results, and arrange for
        // run() to return (if still active).
        // Ignoring query cancelltion does not do any direct harm, but wastes CPU cycles: any
        // results that are pushed once a query is cancelled are ignored anyway.
        // The main purpose of this callback to give queries a chance to stop doing whatever
        // work may still be in progress on a query. Note that cancellations are frequent;
        // not responding to cancelled() correctly causes loss of performance.

        cerr << "query for \"" << scope_name_ << ":" << query_ << "\" cancelled" << endl;
    }

    virtual void run(ReplyProxy const& reply) override
    {
        // The query can do anything it likes with this method, that is, run() can push results
        // directly on the provided reply, or it can save the reply for later use and return from
        // run(). It is OK to push results on the reply from a different thread.
        // The only obligation on run() is that, if cancelled() is called, and run() is still active
        // at that time, run() must tidy up and return in a timely fashion.
        add_func_(query_, reply);
    }

private:
    string scope_name_;
    string query_;
    std::function<void(string const&, ReplyProxy const&)> add_func_;
};

// Example scope D: replies asynchronously to queries.
// The scope's run() method is used as a worker thread that pulls queries from a queue.
// The MyQuery object's run() method adds the query string and the reply proxy to the queue
// and signals the worker thread, and then returns. The worker thread pushes the results.

class MyScope : public ScopeBase
{
public:
    virtual int start(RegistryProxy const&) override
    {
        return VERSION;
    }

    virtual void stop() override
    {
        queue.finish();
        done.store(true);
    }

    virtual void run() override
    {
        // What run() does is up to the scope. For example, we could set up and run an event loop here.
        // It's OK for run() to be empty and return immediately, or to take as long it likes to complete.
        // The only obligation is that, if the scopes run time calls stop(), run() must tidy up and return
        // in as timely a manner as possible.
        while (!done.load())
        {
            string query;
            ReplyProxy reply_proxy;
            if (queue.get(query, reply_proxy) && !done.load())
            {
                for (int i = 0; i < 4; ++i)
                {
                    if (!reply_proxy->push("scope-D: result " + to_string(i) + " for query \"" + query + "\""))
                    {
                        break; // Query was cancelled
                    }
                    sleep(1);
                }
                cerr << "scope-D: query \"" << query << "\" complete" << endl;
            }
        }
    }

    void add_query(string const& query, ReplyProxy const& reply_proxy)
    {
        queue.put(query, reply_proxy);
    }

    virtual QueryBase::UPtr create_query(string const& q) override
    {
        auto add_func = [this](string const& q, ReplyProxy const& p) { this->add_query(q, p); };
        QueryBase::UPtr query(new MyQuery("scope-D", q, add_func));  // TODO: scope name should come from the run time
        cerr << "scope-D: created query: \"" << q << "\"" << endl;
        return query;
    }

    MyScope()
        : done(false)
    {
    }

private:
    Queue queue;
    std::atomic_bool done;
};

// External entry points to allocate and deallocate the scope.

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

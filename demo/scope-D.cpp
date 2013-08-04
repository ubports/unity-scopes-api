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

#include <unity/api/scopes/ScopeBase.h>

#include <condition_variable>
#include <iostream>
#include <list>
#include <mutex>

#define EXPORT __attribute__ ((visibility ("default")))

using namespace std;
using namespace unity::api::scopes;

// Example scope D: replies asynchronously to a query. (Replies are returned after returning from the query() method.)
// Same as scope C, but does not make the call to finished() to show that this happens automatically when
// the reply proxy goes out of scope.

class MyScope : public ScopeBase
{
public:
    virtual int start(RegistryProxy::SPtr const&) override { return VERSION; }
    virtual void stop() override {}
    virtual void run() override
    {
        for (;;)
        {
            std::unique_lock<std::mutex> lock(mutex_);
            condvar_.wait(lock, [this]{ return !queries_.empty(); });
            auto qd = queries_.front();
            queries_.pop_front();
            qd.reply_proxy->send("scope-D: result 1 for query \"" + qd.query + "\"");
            qd.reply_proxy->send("scope-D: result 2 for query \"" + qd.query + "\"");
            qd.reply_proxy->send("scope-D: result 3 for query \"" + qd.query + "\"");
            qd.reply_proxy->send("scope-D: result 4 for query \"" + qd.query + "\"");
            // qd.reply_proxy->finished();
            // Call to finished() commented out here, just to show that it happens automatically.

            cout << "scope-D: query \"" << qd.query << "\" complete" << endl;
        }
    }

    virtual void query(string const& q, ReplyProxy::SPtr const& reply) override
    {
        cout << "scope-D: received query string \"" << q << "\"" << endl;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            QueryData qd = { q, reply };
            queries_.push_back(qd);
        }
        condvar_.notify_one();
    }

private:
    struct QueryData
    {
        std::string query;
        ReplyProxy::SPtr reply_proxy;
    };
    std::list<QueryData> queries_;
    std::mutex mutex_;
    std::condition_variable condvar_;
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

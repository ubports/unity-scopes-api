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

#include <unity/scopes/ScopeBase.h>

#include "SlowCreateScope.h"

#include <condition_variable>
#include <mutex>
#include <thread>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <gtest/gtest.h>
#pragma GCC diagnostic pop

using namespace std;
using namespace unity::scopes;

namespace
{

class TestQuery : public SearchQueryBase
{
public:
    TestQuery(CannedQuery const& query, SearchMetadata const& metadata)
        : SearchQueryBase(query, metadata)
    {
        lock_guard<mutex> lock(mutex_);
        cancelled_ = false;
    }

    virtual void cancelled() override
    {
        lock_guard<mutex> lock(mutex_);
        cancelled_ = true;
        cond_.notify_all();
    }

    virtual void run(SearchReplyProxy const&) override
    {
        if (!valid())
        {
            return; // Query was cancelled already
        }
        // We time out the wait because, otherwise, the scope's object adapter can't shut down
        // and the test hangs forever.
        // If this fails, the cancelled method wasn't called after five seconds.
        auto stop_time = chrono::steady_clock::now() + chrono::seconds(5);
        unique_lock<mutex> lock(mutex_);
        EXPECT_TRUE(cond_.wait_until(lock, stop_time, [this]{ return cancelled_;}));
    }

private:
    mutex mutex_;
    condition_variable cond_;
    bool cancelled_;
};

}  // namespace

SearchQueryBase::UPtr SlowCreateScope::search(CannedQuery const& query, SearchMetadata const& metadata)
{
    // Sleep for a while. This allows the client to call cancel() before this function
    // returns, while the test still holds a fake QueryCtrl returned by the async
    // invocation. When this method returns, the client-side run time calls
    // the real cancel, which triggers the cancelled() callback above, which, in turn,
    // causes TestQuery::run() to complete.
    this_thread::sleep_for(chrono::milliseconds(250));

    return SearchQueryBase::UPtr(new TestQuery(query, metadata));
}

PreviewQueryBase::UPtr SlowCreateScope::preview(Result const&, ActionMetadata const&)
{
    return nullptr;  // Not called
}

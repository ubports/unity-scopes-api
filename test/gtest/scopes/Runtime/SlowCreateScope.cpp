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

#include <gtest/gtest.h>

using namespace std;
using namespace unity::scopes;

namespace
{

class TestQuery : public SearchQueryBase
{
public:
    TestQuery()
    {
        lock_guard<mutex> lock(mutex_);
        cancelled_ = false;
    }

    virtual void cancelled() override
    {
        lock_guard<mutex> lock(mutex_);
        cancelled_ = true;
        cond_.notify_one();
    }

    virtual void run(SearchReplyProxy const&) override
    {
        unique_lock<mutex> lock(mutex_);
        EXPECT_FALSE(cancelled_);
        cond_.wait(lock, [this]{ return cancelled_;});
    }

private:
    mutex mutex_;
    condition_variable cond_;
    bool cancelled_;
};

}  // namespace

int SlowCreateScope::start(string const&, RegistryProxy const &)
{
    return VERSION;
}

void SlowCreateScope::stop()
{
}

void SlowCreateScope::run()
{
}

SearchQueryBase::UPtr SlowCreateScope::search(CannedQuery const&, SearchMetadata const &)
{
    // Sleep for a while. This allows the client to call cancel() before this function
    // returns, while the test still holds a fake QueryCtrl returned by the async
    // invocation. When this method returns, the client-side run time calls
    // the real cancel, which triggers the cancelled() callback above, which, in turn,
    // causes TestQuery::run() to complete.
    this_thread::sleep_for(chrono::milliseconds(200));

    return SearchQueryBase::UPtr(new TestQuery());
}

PreviewQueryBase::UPtr SlowCreateScope::preview(Result const&, ActionMetadata const &)
{
    return nullptr;  // Not called
}

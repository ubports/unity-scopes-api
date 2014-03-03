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

// You may also include individual headers if you prefer.
#include <unity-scopes.h>

#include <condition_variable>
#include <cstdlib>
#include <string.h>
#include <sstream>
#include <iostream>
#include <mutex>
#include <cassert>
#include <unistd.h>

using namespace std;
using namespace unity::scopes;

class Receiver : public SearchListenerBase
{
public:
    Receiver()
        : query_complete_(false)
    {
    }

    virtual void push(CategorisedResult result) override
    {
        cout << "received result: uri=" << result.uri() << endl;
    }

    virtual void finished(ListenerBase::Reason reason, string const& /* error_message */) override
    {
        cout << "query complete, status: " << to_string(reason) << endl;
        lock_guard<decltype(mutex_)> lock(mutex_);
        query_complete_ = true;
        condvar_.notify_one();
    }

    void wait_until_finished()
    {
        unique_lock<decltype(mutex_)> lock(mutex_);
        condvar_.wait(lock, [this] { return this->query_complete_; });
    }

private:
    bool query_complete_;
    mutex mutex_;
    condition_variable condvar_;
};


int main()
{
    try
    {
        Runtime::UPtr rt = Runtime::create(DEMO_RUNTIME_PATH);

        try
        {
            rt->registry();
            assert(false);
        }
        catch (ConfigException const& e)
        {
            cerr << e.what() << endl;
        }

        // Create a proxy to scope-A from a string.
        // Syntax: ipc://<endpoint>#<identity>!<timeout>!<category>
        //
        // "ipc://"         - prefix for all proxies
        // "/tmp/scope-A    - endpoint at which the scope listens
        // "scope-A         - scope name (must be the same as last component of endpoint path)
        // "t=300"          - optional timeout (infinite if not specified)
        // "c=Scope"        - type of proxy (must be "c=Scope")

        Proxy p = rt->string_to_proxy("ipc:///tmp/scope-A#scope-A!t=300!c=Scope");
        assert(p);

        // The returned proxy is of type Proxy and must be down-cast to a scope proxy:
        ScopeProxy sp = dynamic_pointer_cast<Scope>(p);
        assert(sp);

        // We can now use the returned proxy to create a query.
        shared_ptr<Receiver> reply(new Receiver);
        SearchMetadata metadata("C", "desktop");
        metadata.set_cardinality(10);
        auto ctrl = sp->search("query string", metadata, reply); // May raise TimeoutException
        cout << "client: created query" << endl;
        reply->wait_until_finished();
        cout << "client: wait returned" << endl;
    }
    catch (std::exception const& e)
    {
        cerr << e.what() << endl;
        return 1;
    }

    return 0;
}

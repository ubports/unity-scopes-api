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

#include <unity/api/scopes/QueryCtrl.h>
#include <unity/api/scopes/Registry.h>
#include <unity/api/scopes/ReplyBase.h>
#include <unity/api/scopes/Runtime.h>
#include <unity/api/scopes/ResultItem.h>
#include <unity/UnityExceptions.h>

#include <condition_variable>
#include <iostream>
#include <mutex>
#include <unistd.h>

using namespace std;
using namespace unity::api::scopes;

class Reply : public ReplyBase
{
public:
    virtual void push(ResultItem const& result) override
    {
        cout << "received result: " << result << endl;
    }

    virtual void finished() override
    {
        cout << "query complete" << endl;
        unique_lock<decltype(mutex_)> lock(mutex_);
        query_complete_ = true;
        condvar_.notify_one();
    }

    void wait_until_finished()
    {
        unique_lock<decltype(mutex_)> lock(mutex_);
        condvar_.wait(lock, [this] { return this->query_complete_; });
    }

    Reply() :
        query_complete_(false)
    {
    }

private:
    bool query_complete_;
    mutex mutex_;
    condition_variable condvar_;
};

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        cerr << "usage: ./client <scope-letter> query" << endl;
        cerr << "For example: ./client B iron" << endl;
        return 1;
    }

    string scope_name = string("scope-") + argv[1];
    string search_string = argv[2];

    try
    {
        Runtime::UPtr rt = Runtime::create("dash");

        RegistryProxy r = rt->registry();
        ScopeProxy s = r->find(scope_name);
        shared_ptr<Reply> reply(new Reply);
        VariantMap vm;
        vm["cardinality"] = 10;
        vm["locale"] = "C";
        s->create_query(search_string, vm, reply);     // Returns immediately
        cerr << "client: created query" << endl;
        reply->wait_until_finished();
        cerr << "client: wait returned" << endl;
    }

    catch (unity::Exception const& e)
    {
        cerr << e.to_string() << endl;
        return 1;
    }

    return 0;
}

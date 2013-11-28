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

#include <scopes/QueryCtrl.h>
#include <scopes/Registry.h>
#include <scopes/ReceiverBase.h>
#include <scopes/Runtime.h>
#include <scopes/ResultItem.h>
#include <scopes/CategoryRenderer.h>
#include <unity/UnityExceptions.h>

#include <condition_variable>
#include <iostream>
#include <mutex>
#include <unistd.h>

using namespace std;
using namespace unity::api::scopes;

class Receiver : public ReceiverBase
{
public:

    virtual void push(Category::SCPtr category) override
    {
        cout << "received category: id=" << category->id() << " title=" << category->title() << " icon=" << category->icon() << " template=" <<
            category->renderer_template().data() << endl;
    }

    virtual void push(ResultItem result) override
    {
        cout << "received result: uri=" << result.uri() << " title=" << result.title() << " category id: " << result.category()->id() << endl;
    }

    virtual void finished(ReceiverBase::Reason reason) override
    {
        cout << "query complete, status: " << to_string(reason) << endl;
        {
            unique_lock<decltype(mutex_)> lock(mutex_);
            query_complete_ = true;
        }
        condvar_.notify_one();
    }

    void wait_until_finished()
    {
        unique_lock<decltype(mutex_)> lock(mutex_);
        condvar_.wait(lock, [this] { return this->query_complete_; });
    }

    Receiver() :
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
        auto meta = r->find(scope_name);
        shared_ptr<Receiver> reply(new Receiver);
        VariantMap vm;
        vm["cardinality"] = 10;
        vm["locale"] = "C";
        auto ctrl = meta.proxy()->create_query(search_string, vm, reply);     // Returns (almost) immediately
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

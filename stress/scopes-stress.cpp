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

#include <boost/filesystem.hpp>
#include <condition_variable>
#include <cstdlib>
#include <string.h>
#include <sstream>
#include <thread>
#include <iostream>
#include <mutex>
#include <chrono>
#include <cassert>
#include <unistd.h>

using namespace std;
using namespace unity::scopes;

// output variant in a json-like format; note, it doesn't do escaping etc.,
// so the output is not suitable input for a json parser, it's only for
// debugging purposes.
std::string to_string(Variant const& var)
{
    std::ostringstream str;
    switch (var.which())
    {
        case Variant::Type::Int:
            str << var.get_int();
            break;
        case Variant::Type::Null:
            str << "null";
            break;
        case Variant::Type::Bool:
            str << std::boolalpha << var.get_bool();
            break;
        case Variant::Type::String:
            str << "\"" << var.get_string() << "\"";
            break;
        case Variant::Type::Double:
            str << var.get_double();
            break;
        case Variant::Type::Dict:
            str << "{";
            for (auto kv: var.get_dict())
            {
                str << "\"" << kv.first << "\":" << to_string(kv.second) << ", ";
            }
            str << "}";
            break;
        case Variant::Type::Array:
            str << "[";
            for (auto v: var.get_array())
            {
                str << to_string(v) << ",";
            }
            str << "]";
            break;
         default:
            assert(0);
    }
    return str.str();
}

class Receiver : public SearchListenerBase
{
public:
    Receiver(const std::string& scope_id)
        : query_complete_(false),
          push_result_count_(0),
          scope_id_(scope_id)
    {
    }

    virtual void push(Department::SCPtr const& parent) override
    {
    }

    virtual void push(Category::SCPtr const& category) override
    {
    }

    virtual void push(CategorisedResult result) override
    {
        cout << scope_id_ << " received: " << result.uri() << std::endl;
        push_result_count_++;
    }

    virtual void push(experimental::Annotation annotation) override
    {
    }

    void push(Filters const& /* filters */, FilterState const& /* filter_state */) override
    {
    }

    virtual void finished(CompletionDetails const& details) override
    {
        cout << "query complete, status: " << to_string(details.status());
        if (details.status() == CompletionDetails::Error)
        {
            cout << ": " << details.message();
        }
        cout << endl;
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

    int result_count() const
    {
        return push_result_count_;
    }

    std::string scope_id() const
    {
        return scope_id_;
    }

private:
    bool query_complete_;
    int push_result_count_ = 0;
    mutex mutex_;
    condition_variable condvar_;
    std::string scope_id_;
};

int main(int argc, char* argv[])
{
    std::list<std::string> scopes {"scope1", "scope2", "scope3"};

    try
    {
        Runtime::UPtr rt;
        // use Runtime.ini from the current directory if present, otherwise let the API pick the default one
        const boost::filesystem::path path("Runtime.ini");
        if (boost::filesystem::exists(path))
        {
            rt = Runtime::create(path.native());
        }
        else
        {
            rt = Runtime::create();
        }
        RegistryProxy r = rt->registry();

        auto scopes_meta = r->list();
        cout << "Scopes:" << scopes_meta.size() << endl;

        for (int i = 0; i<1000; i++)
        {
            cout << "---------------------------- RUN #" << i << "-----------------------------" << endl;
            list<shared_ptr<Receiver>> replies;
            for (auto scope_id: scopes)
            {
                auto meta = r->get_metadata(scope_id);
                shared_ptr<Receiver> reply(new Receiver(scope_id));
                replies.push_back(reply);

                FilterState filter_state;
                SearchMetadata metadata("C", "phone");
                auto ctrl = meta.proxy()->search("", "", filter_state, metadata, reply); // May raise TimeoutException
                cout << "sent query to scope " << scope_id << endl;
            }

            cout << "waiting for replies to finish" << endl;
            for (auto reply: replies)
            {
                reply->wait_until_finished();
                cout << "finished query for scope " << reply->scope_id() << ", num of pushes=" << reply->result_count() << endl;
                if (reply->result_count() != 50)
                {
                    cerr << "!!!!!!!!!!!!!!!!!!!  ERROR !!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
                    return 1;
                }
            }

            this_thread::sleep_for(chrono::seconds(1));
        }
    }
    catch (unity::Exception const& e)
    {
        cerr << e.to_string() << endl;
        return 1;
    }

    return 0;
}

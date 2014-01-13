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

#include <scopes/CategoryRenderer.h>
#include <scopes/QueryCtrl.h>
#include <scopes/Registry.h>
#include <scopes/ListenerBase.h>
#include <scopes/Runtime.h>
#include <scopes/CategorisedResult.h>
#include <scopes/CategoryRenderer.h>
#include <scopes/ScopeExceptions.h>
#include <scopes/ActivationResponse.h>
#include <unity/UnityExceptions.h>

#include <condition_variable>
#include <cstdlib>
#include <string.h>
#include <iostream>
#include <mutex>
#include <unistd.h>

using namespace std;
using namespace unity::api::scopes;

class Receiver : public SearchListener
{
public:
    Receiver(int index_to_activate)
        : index_to_activate_(index_to_activate),
          push_result_count_(0)
    {
    }

    virtual void push(Category::SCPtr category) override
    {
        cout << "received category: id=" << category->id()
             << " title=" << category->title()
             << " icon=" << category->icon()
             << " template=" << category->renderer_template().data()
             << endl;
    }

    virtual void push(CategorisedResult result) override
    {
        cout << "received result: uri=" << result.uri()
             << " title=" << result.title()
             << " category id: "
             << result.category()->id()
             << endl;
        ++push_result_count_;
        if (index_to_activate_ > 0 && push_result_count_ == index_to_activate_)
        {
            activate_result_ = std::make_shared<Result>(result);
        }
    }

    virtual void push(Annotation annotation) override
    {
        auto links = annotation.links();
        cout << "received annotation of type " << annotation.annotation_type()
             << " with " << links.size() << " link(s):"
             << endl;
        for (auto link: links)
        {
            cout << "  " << link->query().to_string() << endl;
        }
    }

    virtual void finished(ListenerBase::Reason reason, string const& error_message) override
    {
        cout << "query complete, status: " << to_string(reason);
        if (reason == ListenerBase::Error)
        {
            cout << ": " << error_message;
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

    std::shared_ptr<Result> result_to_activate() const
    {
        return activate_result_;
    }

    Receiver() :
        query_complete_(false)
    {
    }

private:
    bool query_complete_;
    int index_to_activate_;
    mutex mutex_;
    condition_variable condvar_;
    int push_result_count_ = 0;
    std::shared_ptr<Result> activate_result_;
};

class ActivationReceiver : public ActivationListener
{
public:
    void activation_response(ActivationResponse const& response) override
    {
        cout << "\tGot activation response: " << response.status() << endl;
    }

    void finished(Reason r, std::string const& error_message)
    {
        cout << "\tActivation finished, reason: " << r << ", error_message: " << error_message << endl;
        condvar_.notify_one();
    }

    void wait_until_finished()
    {
        unique_lock<decltype(mutex_)> lock(mutex_);
        condvar_.wait(lock);
    }

private:
    mutex mutex_;
    condition_variable condvar_;
};

void print_usage()
{
    cerr << "usage: ./client <scope-letter> query [activate n]" << endl;
    cerr << "For example: ./client B iron" << endl;
    cerr << "         or: ./client B iron activate 1" << endl;
    exit(1);
}

int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        print_usage();
    }

    string scope_name = string("scope-") + argv[1];
    string search_string = argv[2];
    int activate_result_index = 0; //the default index of 0 won't activate

    // poor man's getopt
    if (argc > 3)
    {
        if (argc == 5)
        {
            if (strcmp(argv[3], "activate") == 0)
            {
                activate_result_index = atoi(argv[4]);
            }
            else
            {
                print_usage();
            }
        }
        else
        {
            print_usage();
        }
    }

    try
    {
        Runtime::UPtr rt = Runtime::create("Runtime.ini");

        RegistryProxy r = rt->registry();
        auto meta = r->get_metadata(scope_name);
        cout << "Scope metadata:   " << endl;
        cout << "\tscope_name:     " << meta.scope_name() << endl;
        cout << "\tdisplay_name:   " << meta.display_name() << endl;
        cout << "\tdescription:    " << meta.description() << endl;
        string tmp;
        try
        {
            tmp = meta.art();
            cout << "\tart:            " << meta.art() << endl;
        }
        catch (NotFoundException const& e)
        {
        }
        try
        {
            tmp = meta.icon();
            cout << "\ticon:           " << meta.icon() << endl;
        }
        catch (NotFoundException const& e)
        {
        }
        try
        {
            tmp = meta.search_hint();
            cout << "\tsearch_hint:    " << meta.search_hint() << endl;
        }
        catch (NotFoundException const& e)
        {
        }
        try
        {
            tmp = meta.hot_key();
            cout << "\thot_key:        " << meta.hot_key() << endl;
        }
        catch (NotFoundException const& e)
        {
        }
        shared_ptr<Receiver> reply(new Receiver(activate_result_index));
        VariantMap vm;
        vm["cardinality"] = 10;
        vm["locale"] = "C";
        auto ctrl = meta.proxy()->create_query(search_string, vm, reply);     // Returns (almost) immediately
        cout << "client: created query" << endl;
        reply->wait_until_finished();
        cout << "client: wait returned" << endl;

        // handle activation
        if (activate_result_index > 0)
        {
            shared_ptr<ActivationReceiver> act_reply(new ActivationReceiver);
            auto result = reply->result_to_activate();
            if (result != nullptr)
            {
                cout << "client: activating result item #" << activate_result_index << ", uri:" << result->uri() << endl;
                bool direct_activation = result->direct_activation();
                cout << "\tdirect activation: " << direct_activation << endl;
                if (!direct_activation)
                {
                    auto target_scope = result->activation_scope_name();
                    ScopeProxy proxy;
                    if (target_scope == meta.scope_name())
                    {
                        proxy = meta.proxy();
                    }
                    else
                    {
                        meta = r->get_metadata(target_scope);
                    }
                    cout << "\tactivation scope name: " << target_scope << endl;
                    proxy->activate(*result, vm, act_reply);
                    act_reply->wait_until_finished();
                }
            }
            else
            {
                cout << "Nothing to activate! Invalid result index?" << endl;
            }
        }
    }

    catch (unity::Exception const& e)
    {
        cerr << e.to_string() << endl;
        return 1;
    }

    return 0;
}

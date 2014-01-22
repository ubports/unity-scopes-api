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

#include <unity/scopes/CategoryRenderer.h>
#include <unity/scopes/QueryCtrl.h>
#include <unity/scopes/Registry.h>
#include <unity/scopes/ListenerBase.h>
#include <unity/scopes/Runtime.h>
#include <unity/scopes/CategorisedResult.h>
#include <unity/scopes/CategoryRenderer.h>
#include <unity/scopes/ScopeExceptions.h>
#include <unity/scopes/ActivationResponse.h>
#include <unity/UnityExceptions.h>

#include <condition_variable>
#include <cstdlib>
#include <string.h>
#include <iostream>
#include <mutex>
#include <unistd.h>

using namespace std;
using namespace unity::scopes;

class Receiver : public SearchListener
{
public:
    Receiver(int index_to_save)
        : index_to_save_(index_to_save),
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
        if (index_to_save_ > 0 && push_result_count_ == index_to_save_)
        {
            saved_result_ = std::make_shared<Result>(result);
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

    std::shared_ptr<Result> saved_result() const
    {
        return saved_result_;
    }

    int result_count() const
    {
        return push_result_count_;
    }

    Receiver() :
        query_complete_(false)
    {
    }

private:
    bool query_complete_;
    int index_to_save_;
    mutex mutex_;
    condition_variable condvar_;
    int push_result_count_ = 0;
    std::shared_ptr<Result> saved_result_;
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

class PreviewReceiver : public PreviewListener
{
public:
    void push(PreviewWidgetList const& widgets) override
    {
        cout << "\tGot preview widgets:" << endl;
        for (auto it = widgets.begin(); it != widgets.end(); ++it)
        {
            cout << "\t\t" << it->data();
            cout << endl;
        }
    }

    void push(std::string const& key, Variant const& value) override
    {
        cout << "\tPushed preview data: \"" << key << "\", value: ";
        if (value.which() == Variant::Type::String)
        {
            cout << value.get_string();
        }
        else if (value.which() == Variant::Type::Null)
        {
            cout << "(null)";
        }
        else
        {
            cout << "(non-string value)";
        }
        cout << endl;
    }

    void finished(Reason r, std::string const& error_message) override
    {
        cout << "\tPreview finished, reason: " << r << ", error_message: " << error_message << endl;
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

enum class ResultOperation
{
    None,
    Activation,
    Preview
};

int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        print_usage();
    }

    string scope_name = string("scope-") + argv[1];
    string search_string = argv[2];
    int result_index = 0; //the default index of 0 won't activate
    ResultOperation result_op = ResultOperation::None;

    // poor man's getopt
    if (argc > 3)
    {
        if (argc == 5)
        {
            if (strcmp(argv[3], "activate") == 0)
            {
                result_index = atoi(argv[4]);
                result_op = ResultOperation::Activation;
            }
            else if (strcmp(argv[3], "preview") == 0)
            {
                result_index = atoi(argv[4]);
                result_op = ResultOperation::Preview;
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
        shared_ptr<Receiver> reply(new Receiver(result_index));
        VariantMap vm;
        vm["cardinality"] = 10;
        vm["locale"] = "C";
        auto ctrl = meta.proxy()->create_query(search_string, vm, reply); // May raise TimeoutException
        cout << "client: created query" << endl;
        reply->wait_until_finished();
        cout << "client: wait returned" << endl;

        // handle activation
        if (result_index > 0)
        {
            auto result = reply->saved_result();
            if (!result)
            {
                cout << "Nothing to activate! Requested result with index " << result_index << " but got " << reply->result_count() << " result(s) only" << endl;
                return 1;
            }
            if (result_op == ResultOperation::Activation)
            {
                shared_ptr<ActivationReceiver> act_reply(new ActivationReceiver);
                cout << "client: activating result item #" << result_index << ", uri:" << result->uri() << endl;
                bool direct_activation = result->direct_activation();
                cout << "\tdirect activation: " << direct_activation << endl;
                if (!direct_activation)
                {
                    auto target_scope = result->activation_scope_name();
                    ScopeProxy proxy;
                    if (target_scope != meta.scope_name()) // if activation scope is different than current, get the right proxy
                    {
                        meta = r->get_metadata(target_scope);
                    }
                    proxy = meta.proxy();
                    cout << "\tactivation scope name: " << target_scope << endl;
                    proxy->activate(*result, vm, act_reply);
                    act_reply->wait_until_finished();
                }
            }
            else if (result_op == ResultOperation::Preview)
            {
                shared_ptr<PreviewReceiver> preview_reply(new PreviewReceiver);
                cout << "client: previewing result item #" << result_index << ", uri:" << result->uri() << endl;
                meta.proxy()->preview(*result, vm, preview_reply);
                preview_reply->wait_until_finished();
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

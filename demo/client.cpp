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
#include <iostream>
#include <mutex>
#include <cassert>
#include <unistd.h>

using namespace std;
using namespace unity::scopes;

// conver filter to a string
std::string to_string(FilterBase const& filter)
{
    std::ostringstream str;
    auto const ftype = filter.filter_type();
    cout << "filter id=" << filter.id() << endl;
    if (ftype == "option_selector")
    {
        auto const& selfilter = dynamic_cast<OptionSelectorFilter const&>(filter);
        str << "OptionSelectorFilter" << endl;
        str << " label: " << selfilter.label() << endl;
        str << " multi-select: " << selfilter.multi_select() << endl;
        str << " options:" << endl;
        for (auto op: selfilter.options())
        {
            str << "    id: " << op->id() << ", label: " << op->label() << endl;
        }
    }
    else
    {
        str << "Unknown filter type: " << ftype;
    }
    return str.str();
}

std::string to_string(Department::SCPtr const& dep, std::string const& indent = "")
{
    std::ostringstream str;
    str << indent << "department id=" << dep->id() << ", name=" << dep->label() << ", has_children=" << dep->has_subdepartments() << endl;
    auto const subdeps = dep->subdepartments();
    if (!subdeps.empty())
    {
        str << indent << "\tsubdepartments:" << endl;
        for (auto const& d: subdeps)
        {
            str << indent << to_string(d, indent + "\t\t");
        }
    }
    return str.str();
}

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

std::string to_string(PreviewWidget const& widget, std::string const& indent = "")
{
    std::ostringstream str;
    str << indent << "widget: id=" << widget.id() << ", type=" << widget.widget_type() << endl
        << indent << "attributes: " << to_string(Variant(widget.attribute_values())) << endl
        << indent << "components: {";
    for (const auto kv: widget.attribute_mappings())
    {
        str << "\"" << kv.first << "\": \"" << kv.second << "\", ";
    }
    str << "}" << endl;
    if (widget.widget_type() == "expandable")
    {
        str << indent << "\twidgets = {";
        for (const auto w: widget.widgets())
        {
            str << to_string(w, indent + "\t\t");
        }
        str << "}" << std::endl;
    }
    return str.str();
}

class Receiver : public SearchListenerBase
{
public:
    Receiver(int index_to_save)
        : query_complete_(false),
          push_result_count_(0),
          index_to_save_(index_to_save)
    {
    }

    virtual void push(Department::SCPtr const& parent) override
    {
        cout << "\treceived departments:" << endl;
        cout << to_string(parent);
    }

    virtual void push(Category::SCPtr const& category) override
    {
        cout << "received category: id=" << category->id()
             << " title=" << category->title()
             << " icon=" << category->icon()
             << " template=" << category->renderer_template().data()
             << endl;
    }

    virtual void push(CategorisedResult result) override
    {
        VariantMap result_dict(result.serialize());
        result_dict = result_dict["attrs"].get_dict();
        cout << "received result: category id=" << result.category()->id() << endl << "{" << endl;
        for (auto attr_it = result_dict.begin(); attr_it != result_dict.end(); ++attr_it)
        {
            cout << "\t\"" << attr_it->first << "\": " << attr_it->second.serialize_json();
        }
        cout << "}" << endl;
        ++push_result_count_;
        if (index_to_save_ > 0 && push_result_count_ == index_to_save_)
        {
            saved_result_ = std::make_shared<Result>(result);
        }
    }

    virtual void push(experimental::Annotation annotation) override
    {
        auto links = annotation.links();
        cout << "received annotation of type " << annotation.annotation_type()
             << " with " << links.size() << " link(s):"
             << endl;
        for (auto link: links)
        {
            cout << "  " << link->query().to_uri() << endl;
        }
    }

    void push(Filters const& filters, FilterState const& /* filter_state */) override
    {
        cout << "received " << filters.size() << " filters" << endl;
        for (auto f: filters)
        {
            cout << to_string(*f) << endl;
        }
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

    std::shared_ptr<Result> saved_result() const
    {
        return saved_result_;
    }

    int result_count() const
    {
        return push_result_count_;
    }

private:
    bool query_complete_;
    int push_result_count_ = 0;
    int index_to_save_;
    std::shared_ptr<Result> saved_result_;
    mutex mutex_;
    condition_variable condvar_;
};

class ActivationReceiver : public ActivationListenerBase
{
public:
    ActivationReceiver()
        : query_complete_(false)
    {
    }

    void activated(ActivationResponse const& response) override
    {
        cout << "\tGot activation response: " << response.status() << endl;
    }

    void finished(CompletionDetails const& details)
    {
        cout << "\tActivation finished, status: " << to_string(details.status()) << ", message: " << details.message() << endl;
        lock_guard<decltype(mutex_)> lock(mutex_);
        query_complete_ = true;
        condvar_.notify_one();
    }

    void wait_until_finished()
    {
        unique_lock<decltype(mutex_)> lock(mutex_);
        condvar_.wait(lock, [this](){ return query_complete_; });
    }

private:
    bool query_complete_;
    mutex mutex_;
    condition_variable condvar_;
};

class PreviewReceiver : public PreviewListenerBase
{
public:
    PreviewReceiver()
        : query_complete_(false)
    {
    }

    void push(ColumnLayoutList const& columns) override
    {
        cout << "\tGot column layouts:" << endl;
        for (auto const& col: columns)
        {
            cout << "\t\tLayout for " << col.size() << " column(s):" << endl;
            for (int i = 0; i<col.size(); i++)
            {
                cout << "\t\t\tColumn #" << i << ": ";
                for (auto const& w: col.column(i))
                {
                    cout << w << ", ";
                }
                cout << endl;
            }

        }
    }

    void push(PreviewWidgetList const& widgets) override
    {
        cout << "\tGot preview widgets:" << endl;
        for (auto it = widgets.begin(); it != widgets.end(); ++it)
        {
            cout << to_string(*it) << endl;
        }
    }

    void push(std::string const& key, Variant const& value) override
    {
        cout << "\tPushed preview data: \"" << key << "\", value: ";
        cout << to_string(value) << endl;
    }

    void finished(CompletionDetails const& details) override
    {
        lock_guard<decltype(mutex_)> lock(mutex_);
        cout << "\tPreview finished, status: " << to_string(details.status()) << ", message: " << details.message() << endl;
        query_complete_ = true;
        condvar_.notify_one();
    }

    void wait_until_finished()
    {
        unique_lock<decltype(mutex_)> lock(mutex_);
        condvar_.wait(lock, [this](){ return query_complete_; });
    }

private:
    bool query_complete_;
    mutex mutex_;
    condition_variable condvar_;
};

void print_usage()
{
    cerr << "usage: ./scopes-client <scope-id> query [activate n] | [preview n]" << endl;
    cerr << "   or: ./scopes-client <canned-query> [activate n] | [preview n]" << endl;
    cerr << "   or: ./scopes-client list" << endl;
    cerr << "   canned query format is: scope://<scope-id>?q=<query>&dep=<department-id>&filters=<filter-state-json>" << endl;
    cerr << "For example: ./scopes-client scope-B iron" << endl;
    cerr << "         or: ./scopes-client scope-B iron activate 1" << endl;
    cerr << "         or: ./scopes-client \"scope://scope-A?q=iron\" preview 1" << endl;
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
    int result_index = 0; //the default index of 0 won't activate
    ResultOperation result_op = ResultOperation::None;
    bool do_list = false;
    string scope_id;
    string search_string;
    string department_id;
    FilterState filter_state;

    // poor man's getopt
    int index = 1;
    if (argc == 2 && strcmp(argv[1], "list") == 0)
    {
        do_list = true;
    }
    else if (argc > 1 && strncmp(argv[index], "scope://", 8) == 0)
    {
        auto q = CannedQuery::from_uri(argv[index++]);
        scope_id = q.scope_id();
        search_string = q.query_string();
        department_id = q.department_id();
        filter_state = q.filter_state();
    }
    else if (argc > 2)
    {
        scope_id = argv[index++];
        search_string = argv[index++];
    }
    else
    {
        print_usage();
    }

    if (argc == index + 2)
    {
        if (strcmp(argv[index], "activate") == 0)
        {
            result_index = atoi(argv[++index]);
            result_op = ResultOperation::Activation;
        }
        else if (strcmp(argv[index], "preview") == 0)
        {
            result_index = atoi(argv[++index]);
            result_op = ResultOperation::Preview;
        }
        else
        {
            print_usage();
        }
    }

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

        if (do_list)
        {
            cout << "Scopes:" << endl;
            auto mmap = r->list();
            for (auto meta: mmap)
            {
                cout << "\t" << meta.second.scope_id() << endl;
            }
            return 0;
        }

        auto meta = r->get_metadata(scope_id);
        cout << "Scope metadata:    " << endl;
        cout << "\tscope_id:        " << meta.scope_id() << endl;
        cout << "\tdisplay_name:    " << meta.display_name() << endl;
        cout << "\tdescription:     " << meta.description() << endl;
        cout << "\tappearance attr: " << to_string(Variant(meta.appearance_attributes())) << endl;
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

        SearchMetadata metadata("C", "desktop");
        metadata.set_cardinality(10);
        auto ctrl = meta.proxy()->search(search_string, department_id, filter_state, metadata, reply); // May raise TimeoutException
        cout << "client: created query" << endl;
        reply->wait_until_finished();

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
                    ActionMetadata const metadata("en", "desktop");
                    auto target_scope = result->target_scope_proxy();
                    cout << "\tactivation scope: " << target_scope->to_string() << endl;
                    target_scope->activate(*result, metadata, act_reply);
                    act_reply->wait_until_finished();
                }
            }
            else if (result_op == ResultOperation::Preview)
            {
                ActionMetadata const metadata("en", "desktop");
                shared_ptr<PreviewReceiver> preview_reply(new PreviewReceiver);
                cout << "client: previewing result item #" << result_index << ", uri:" << result->uri() << endl;
                auto target_scope = result->target_scope_proxy();
                cout << "\tactivation scope ID: " << target_scope->to_string() << endl;
                target_scope->preview(*result, metadata, preview_reply);
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

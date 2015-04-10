/*
 * Copyright (C) 2015 Canonical Ltd
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

#include "EchoScope.h"

#include <unity/scopes/ActionMetadata.h>
#include <unity/scopes/CategorisedResult.h>
#include <unity/scopes/OptionSelectorFilter.h>
#include <unity/scopes/PreviewReply.h>
#include <unity/scopes/ScopeBase.h>
#include <unity/scopes/SearchReply.h>

#include <unity/UnityExceptions.h>
#include <unity/util/FileIO.h>

#include <condition_variable>
#include <mutex>
#include <thread>

using namespace std;
using namespace unity;
using namespace unity::scopes;

namespace
{

class TestQuery : public SearchQueryBase
{
public:
    TestQuery(CannedQuery const& query,
              SearchMetadata const& metadata)
        : SearchQueryBase(query, metadata)
    {
    }

    virtual void cancelled() override
    {
    }

    virtual void run(SearchReplyProxy const& reply) override
    {
        string query_string = query().query_string();

        Department::SPtr parent = Department::create("", query(), "parent");
        Department::SPtr child = Department::create(query_string, query(), query_string);
        parent->add_subdepartment(child);
        reply->register_departments(parent);

        auto filter = OptionSelectorFilter::create(query_string, query_string, false);
        filter->add_option("opt", "opt");
        Filters filters;
        filters.push_front(move(filter));
        reply->push(filters, FilterState());

        experimental::Annotation annotation(experimental::Annotation::Type::GroupedLink);
        annotation.set_label(query_string);
        annotation.add_link("label", query());
        reply->push(annotation);

        OperationInfo info(OperationInfo::Unknown);
        reply->info(info);

        auto cat = reply->register_category(query_string, "", "");
        CategorisedResult res(cat);
        res.set_uri("uri");
        res.set_title(query().query_string());
        res.set_intercept_activation();

        if (valid())
        {
            reply->push(res);
        }
    }
};

class TestPreview : public PreviewQueryBase
{
public:
    TestPreview(Result const& result, ActionMetadata const& metadata)
        : PreviewQueryBase(result, metadata)
    {
    }

    virtual void cancelled() override
    {
    }

    virtual void run(PreviewReplyProxy const& reply) override
    {
        ColumnLayout layout(1);
        layout.add_column({"text"});
        reply->register_layout({layout});

        PreviewWidget widget("text", "text");
        widget.add_attribute_value("text", Variant("some text"));
        reply->push({widget});

        reply->push("some_attribute", Variant("some_value"));
    }
};

class TestActivation : public ActivationQueryBase
{
public:
    TestActivation(Result const& result, ActionMetadata const& metadata)
        : ActivationQueryBase(result, metadata)
    {
    }

    virtual ActivationResponse activate() override
    {
        return ActivationResponse(ActivationResponse::NotHandled);
    }
};

}  // namespace

void EchoScope::start(string const& /* scope_id */)
{
}

void EchoScope::stop()
{
}

void EchoScope::run()
{
}

SearchQueryBase::UPtr EchoScope::search(CannedQuery const& query, SearchMetadata const& metadata)
{
    return SearchQueryBase::UPtr(new TestQuery(query, metadata));
}

PreviewQueryBase::UPtr EchoScope::preview(Result const& result, ActionMetadata const& metadata)
{
    return PreviewQueryBase::UPtr(new TestPreview(result, metadata));
}

ActivationQueryBase::UPtr EchoScope::activate(Result const& result, ActionMetadata const& metadata)
{
    return ActivationQueryBase::UPtr(new TestActivation(result, metadata));
}

ActivationQueryBase::UPtr EchoScope::perform_action(Result const& result,
                                                        ActionMetadata const& metadata,
                                                        string const& /* widget_id */,
                                                        string const& /* action_id */)
{
    return ActivationQueryBase::UPtr(new TestActivation(result, metadata));
}

extern "C"
{

    unity::scopes::ScopeBase*
    // cppcheck-suppress unusedFunction
    UNITY_SCOPE_CREATE_FUNCTION()
    {
        return new EchoScope;
    }

    void
    // cppcheck-suppress unusedFunction
    UNITY_SCOPE_DESTROY_FUNCTION(unity::scopes::ScopeBase* scope_base)
    {
        delete scope_base;
    }
}

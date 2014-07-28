/*
 * Copyright (C) 2014 Canonical Ltd
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
 * Authored by: Pawel Stolowski <pawel.stolowski@canonical.com>
 */

#include <unity/scopes/SearchQueryBase.h>
#include <unity/scopes/PreviewQueryBase.h>
#include <unity/scopes/ScopeBase.h>
#include <unity/scopes/CategorisedResult.h>
#include <unity/scopes/PreviewWidget.h>
#include <unity/scopes/SearchReply.h>
#include <unity/scopes/Runtime.h>
#include <unity/scopes/PreviewReply.h>

using namespace std;
using namespace unity::scopes;

class MyQuery : public SearchQueryBase
{
public:
    MyQuery(CannedQuery const& query, SearchMetadata const& metadata) :
        SearchQueryBase(query, metadata)
    {
    }

    virtual void cancelled() override
    {
    }

    virtual void run(SearchReplyProxy const& reply) override
    {
        CategoryRenderer rdr;
        auto cat = reply->register_category("cat1", "Category 1", "", rdr);
        CategorisedResult res(cat);
        res.set_uri("uri");
        reply->push(res);
    }
};

class MyPreview : public PreviewQueryBase
{
public:
    MyPreview(Result const& result, ActionMetadata const& metadata) :
        PreviewQueryBase(result, metadata)
    {
    }

    ~MyPreview()
    {
    }

    virtual void cancelled() override
    {
    }

    virtual void run(PreviewReplyProxy const& reply) override
    {
        PreviewWidgetList widgets;
        widgets.emplace_back(PreviewWidget(R"({"id": "header", "type": "header", "title": "title", "subtitle": "author", "rating": "rating"})"));
        reply->push(widgets);
    }
};

class MyScope : public ScopeBase
{
public:
    virtual void start(string const&) override {}

    virtual void stop() override {}

    virtual SearchQueryBase::UPtr search(CannedQuery const& q, SearchMetadata const& metadata) override
    {
        SearchQueryBase::UPtr query(new MyQuery(q, metadata));
        return query;
    }

    virtual PreviewQueryBase::UPtr preview(Result const& result, ActionMetadata const& metadata) override
    {
        PreviewQueryBase::UPtr preview(new MyPreview(result, metadata));
        return preview;
    }
};

int main(int argc, char** argv)
{
    // Set parameter as "FAIL" if not provided as to intentionally break the scope, hence causing
    // dependant tests to fail (E.g. Registry_test and RegistryI_test).
    std::string unused_arg_1 = argc > 1 ? argv[1] : "FAIL";
    std::string scope_config = argc > 2 ? argv[2] : "FAIL";
    std::string rt_config = argc > 3 ? argv[3] : "FAIL";
    std::string unused_arg_2 = argc > 4 ? argv[4] : "FAIL";

    // In order to test that custom exec splitting works, we check here that our arbitrary arguments
    // have been delivered to us as expected.
    if (unused_arg_1 == "unused arg 1" && unused_arg_2 == "unused arg 2")
    {
        MyScope scope;
        auto runtime = Runtime::create_scope_runtime("testscopeB", rt_config);
        runtime->run_scope(&scope, rt_config, scope_config);
        return 0;
    }
}

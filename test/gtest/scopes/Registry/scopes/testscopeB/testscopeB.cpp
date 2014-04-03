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
    MyPreview(string const& uri) :
        uri_(uri)
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

private:
    string uri_;
};

class MyScope : public ScopeBase
{
public:
    virtual int start(string const&, RegistryProxy const&) override
    {
        return VERSION;
    }

    virtual void stop() override {}

    virtual SearchQueryBase::UPtr search(CannedQuery const&, SearchMetadata const&) override
    {
        SearchQueryBase::UPtr query(new MyQuery());
        return query;
    }

    virtual PreviewQueryBase::UPtr preview(Result const& result, ActionMetadata const&) override
    {
        PreviewQueryBase::UPtr preview(new MyPreview(result.uri()));
        return preview;
    }
};

int main(int /* argc */, char ** /* argv */)
{
    MyScope scope;
    auto runtime = Runtime::create_scope_runtime("testscopeB", TEST_RUNTIME_FILE);
    runtime->run_scope(&scope);
    return 0;
}

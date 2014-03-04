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
 * Authored by: James Henstridge <james.henstridge@canonical.com>
 */

#include <unity/scopes/CannedQuery.h>
#include <unity/scopes/CategorisedResult.h>
#include <unity/scopes/Category.h>
#include <unity/scopes/Department.h>
#include <unity/scopes/internal/PreviewReply.h>
#include <unity/scopes/internal/SearchReply.h>
#include <unity/scopes/Runtime.h>
#include <unity/scopes/ScopeBase.h>

#include "TestScope.h"

using namespace std;
using namespace unity::scopes;

class TestQuery : public SearchQueryBase
{
public:
    TestQuery(CannedQuery const& query)
        : query_(query)
    {
    }

    virtual void cancelled() override
    {
    }
    virtual void run(SearchReplyProxy const& reply) override
    {
        Department dep("news", query_, "News");
        dep.set_subdepartments({{"subdep1", query_, "Europe"},{"subdep2", query_, "US"}});
        reply->register_departments({dep}, "news");

        auto cat = reply->register_category("cat1", "Category 1", "");
        CategorisedResult res(cat);
        res.set_uri("uri");
        res.set_title("title");
        res.set_art("art");
        res.set_dnd_uri("dnd_uri");
        reply->push(res);

        CannedQuery query("scope-A", "foo", "dep1");
        Annotation annotation(Annotation::Type::Link);
        annotation.add_link("Link1", query);
        reply->register_annotation(annotation);
    }

private:
    CannedQuery query_;
};

class TestPreview : public PreviewQueryBase
{
public:
    virtual void cancelled() override
    {
    }
    virtual void run(PreviewReplyProxy const& reply) override
    {
        PreviewWidgetList widgets;
        widgets.emplace_back(PreviewWidget(R"({"id": "header", "type": "header", "title": "title", "subtitle": "author", "rating": "rating"})"));
        widgets.emplace_back(PreviewWidget(R"({"id": "id", "type": "image", "art": "screenshot-url"})"));
        reply->push(widgets);
        reply->push("author", Variant("Foo"));
        reply->push("rating", Variant("Bar"));
    }
};

int TestScope::start(string const&, RegistryProxy const &)
{
    return VERSION;
}

void TestScope::stop()
{
}

void TestScope::run()
{
}

SearchQueryBase::UPtr TestScope::search(CannedQuery const& query, SearchMetadata const &)
{
    return SearchQueryBase::UPtr(new TestQuery(query));
}

PreviewQueryBase::UPtr TestScope::preview(Result const&, ActionMetadata const &)
{
    return PreviewQueryBase::UPtr(new TestPreview());
}

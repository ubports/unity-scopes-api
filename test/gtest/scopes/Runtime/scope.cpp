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

#include <unity/scopes/Category.h>
#include <unity/scopes/CategorisedResult.h>
#include <unity/scopes/SearchReply.h>
#include <unity/scopes/PreviewReply.h>
#include <unity/scopes/Runtime.h>
#include <unity/scopes/ScopeBase.h>
#include <unity/scopes/Query.h>

#include "scope.h"

using namespace std;
using namespace unity::scopes;

class TestQuery : public SearchQuery
{
public:
    virtual void cancelled() override
    {
    }
    virtual void run(SearchReplyProxy const& reply) override
    {
        auto cat = reply->register_category("cat1", "Category 1", "");
        CategorisedResult res(cat);
        res.set_uri("uri");
        res.set_title("title");
        res.set_art("art");
        res.set_dnd_uri("dnd_uri");
        reply->push(res);
    }
};

class TestPreview : public PreviewQuery
{
public:
    virtual void cancelled() override
    {
    }
    virtual void run(PreviewReplyProxy const& reply) override
    {
        PreviewWidgetList widgets;
        widgets.emplace_back(PreviewWidget(R"({"id": "header", "type": "header", "title": "title", "subtitle": "author", "rating": "rating"})"));
        widgets.emplace_back(PreviewWidget(R"({"type": "image", "art": "screenshot-url"})"));
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

QueryBase::UPtr TestScope::create_query(Query const &, VariantMap const &)
{
    return QueryBase::UPtr(new TestQuery());
}

QueryBase::UPtr TestScope::preview(Result const&, VariantMap const &)
{
    return QueryBase::UPtr(new TestPreview());
}

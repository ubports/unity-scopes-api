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

#include <iostream>

#define EXPORT __attribute__ ((visibility ("default")))

using namespace std;
using namespace unity::scopes;

// Example scope A: replies synchronously to a query. (Replies are returned before returning from the run() method.)

class MyQuery : public SearchQueryBase
{
public:
    MyQuery(CannedQuery const& query) :
        query_(query)
    {
    }

    ~MyQuery()
    {
    }

    virtual void cancelled() override
    {
    }

    virtual void run(SearchReplyProxy const& reply) override
    {
        DepartmentList departments({{"news", query_, "News", {{"news-world", query_, "World"}, {"news-europe", query_, "Europe"}}},
                                    {"sport", query_, "Sport"}});
        reply->register_departments(departments);

        Filters filters;
        auto filter = OptionSelectorFilter::create("f1", "Options");
        filter->add_option("1", "Option 1");
        filter->add_option("2", "Option 2");
        filters.push_back(filter);
        FilterState filter_state; // TODO: push real state from query obj
        reply->push(filters, filter_state);

        CategoryRenderer rdr;
        auto cat = reply->register_category("cat1", "Category 1", "", rdr);
        CategorisedResult res(cat);
        res.set_uri("uri");
        res.set_title("scope-A: result 1 for query \"" + query_.query_string() + "\"");
        res.set_art("icon");
        res.set_dnd_uri("dnd_uri");
        reply->push(res);

        CannedQuery q("scope-A", query_.query_string(), "");
        Annotation annotation(Annotation::Type::Link);
        annotation.add_link("More...", q);
        reply->register_annotation(annotation);

        cout << "scope-A: query \"" << query_.query_string() << "\" complete" << endl;
    }

private:
    CannedQuery query_;
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
        widgets.emplace_back(PreviewWidget(R"({"id": "img", "type": "image", "art": "screenshot-url"})"));

        PreviewWidget w("img2", "image");
        w.add_attribute_value("zoomable", Variant(false));
        w.add_attribute_mapping("art", "screenshot-url");
        widgets.emplace_back(w);

        ColumnLayout layout1col(1);
        layout1col.add_column({"header", "title"});

        ColumnLayout layout2col(2);
        layout2col.add_column({"header", "title"});
        layout2col.add_column({"author", "rating"});

        ColumnLayout layout3col(3);
        layout3col.add_column({"header", "title"});
        layout3col.add_column({"author"});
        layout3col.add_column({"rating"});

        reply->register_layout({layout1col, layout2col, layout3col});
        reply->push(widgets);
        reply->push("author", Variant("Foo"));
        reply->push("rating", Variant("4 blah"));
        cout << "scope-A: preview for \"" << uri_ << "\" complete" << endl;
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

    virtual SearchQueryBase::UPtr search(CannedQuery const& q, SearchMetadata const&) override
    {
        SearchQueryBase::UPtr query(new MyQuery(q));
        cout << "scope-A: created query: \"" << q.query_string() << "\"" << endl;
        return query;
    }

    virtual PreviewQueryBase::UPtr preview(Result const& result, ActionMetadata const&) override
    {
        PreviewQueryBase::UPtr preview(new MyPreview(result.uri()));
        cout << "scope-A: created previewer: \"" << result.uri() << "\"" << endl;
        return preview;
    }
};

extern "C"
{

    EXPORT
    unity::scopes::ScopeBase*
    // cppcheck-suppress unusedFunction
    UNITY_SCOPE_CREATE_FUNCTION()
    {
        return new MyScope;
    }

    EXPORT
    void
    // cppcheck-suppress unusedFunction
    UNITY_SCOPE_DESTROY_FUNCTION(unity::scopes::ScopeBase* scope_base)
    {
        delete scope_base;
    }

}

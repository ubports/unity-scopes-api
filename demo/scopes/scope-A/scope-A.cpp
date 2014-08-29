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
    MyQuery(string const& scope_id, CannedQuery const& query, SearchMetadata const& metadata) :
        SearchQueryBase(query, metadata),
        scope_id_(scope_id)
    {
        cerr << scope_id_ << ": query instance for \"" << query.query_string() << "\" created" << endl;
    }

    ~MyQuery()
    {
        cerr << scope_id_ << ": query instance for \"" << query().query_string() << "\" destroyed" << endl;
    }

    virtual void cancelled() override
    {
    }

    virtual void run(SearchReplyProxy const& reply) override
    {
        if (!valid())
        {
            return;  // Query was cancelled
        }

        Department::SPtr all_depts = Department::create("", query(), "All departments");
        Department::SPtr news_dept = Department::create("news", query(), "News");
        news_dept->set_subdepartments({Department::create("news-world", query(), "World"), Department::create("news-europe", query(), "Europe")});
        all_depts->set_subdepartments({news_dept, Department::create("sport", query(), "Sport")});
        reply->register_departments(all_depts);

        Filters filters;
        OptionSelectorFilter::SPtr filter = OptionSelectorFilter::create("f1", "Options");
        filter->add_option("1", "Option 1");
        filter->add_option("2", "Option 2");
        filters.push_back(filter);
        FilterState filter_state; // TODO: push real state from query obj
        if (!reply->push(filters, filter_state))
        {
            return;  // Query was cancelled
        }

        CategoryRenderer rdr;
        auto cat = reply->register_category("cat1", "Category 1", "", rdr);
        CategorisedResult res(cat);
        res.set_uri("uri");
        res.set_title("scope-A: result 1 for query \"" + query().query_string() + "\"");
        res.set_art("icon");
        res.set_dnd_uri("dnd_uri");
        if (!reply->push(res))
        {
            return;  // Query was cancelled
        }

        CannedQuery q("scope-A", query().query_string(), "");
        experimental::Annotation annotation(experimental::Annotation::Type::Link);
        annotation.add_link("More...", q);
        reply->push(annotation);

        cerr << "scope-A: query \"" << query().query_string() << "\" complete" << endl;
    }

private:
    string scope_id_;
};

class MyPreview : public PreviewQueryBase
{
public:
    MyPreview(Result const& result, ActionMetadata const& metadata, string const& scope_id) :
        PreviewQueryBase(result, metadata),
        scope_id_(scope_id)
    {
        cerr << scope_id_ << ": preview instance for \"" << result.uri() << "\" created" << endl;
    }

    ~MyPreview()
    {
        cerr << scope_id_ << ": preview instance for \"" << result().uri() << "\" destroyed" << endl;
    }

    virtual void cancelled() override
    {
    }

    virtual void run(PreviewReplyProxy const& reply) override
    {
        PreviewWidgetList widgets;
        widgets.emplace_back(PreviewWidget(R"({"id": "header", "type": "header", "title": "title", "subtitle": "author", "rating": "rating"})"));

        PreviewWidget expandable("exp", "expandable");
        expandable.add_attribute_value("title", Variant("foo"));
        expandable.add_attribute_value("collapsed-widgets", Variant(0));

        PreviewWidget w1("t1", "text");
        w1.add_attribute_value("title", Variant("widget1"));
        w1.add_attribute_value("text", Variant("fooooaosdoasdoa"));
        PreviewWidget w2("t2", "text");
        w2.add_attribute_value("title", Variant("widget2"));
        w2.add_attribute_mapping("text", "txt");
        expandable.add_widget(w1);
        expandable.add_widget(w2);

        widgets.emplace_back(expandable);

        PreviewWidget w("img2", "image");
        w.add_attribute_value("zoomable", Variant(false));
        w.add_attribute_mapping("art", "screenshot-url");
        widgets.emplace_back(w);

        ColumnLayout layout1col(1);
        layout1col.add_column({"header", "exp"});

        ColumnLayout layout2col(2);
        layout2col.add_column({"header", "exp"});
        layout2col.add_column({"author", "exp"});

        ColumnLayout layout3col(3);
        layout3col.add_column({"header", "exp"});
        layout3col.add_column({"author"});
        layout3col.add_column({"rating"});

        reply->register_layout({layout1col, layout2col, layout3col});
        if (!reply->push(widgets))
        {
            return;  // Query was cancelled
        }
        if (!reply->push("txt", Variant("This is a text")))
        {
            return;
        }
        if (!reply->push("author", Variant("Foo")))
        {
            return;  // Query was cancelled
        }
        if (!reply->push("rating", Variant("4 blah")))
        {
            return;  // Query was cancelled
        }
        cerr << "scope-A: preview for \"" << result().uri() << "\" complete" << endl;
    }

private:
    string scope_id_;
};

class MyScope : public ScopeBase
{
public:
    virtual void start(string const& scope_id) override
    {
        scope_id_ = scope_id;
    }

    virtual void stop() override {}

    virtual SearchQueryBase::UPtr search(CannedQuery const& q, SearchMetadata const& metadata) override
    {
        SearchQueryBase::UPtr query(new MyQuery(scope_id_, q, metadata));
        return query;
    }

    virtual PreviewQueryBase::UPtr preview(Result const& result, ActionMetadata const& metadata) override
    {
        PreviewQueryBase::UPtr preview(new MyPreview(result, metadata, scope_id_));
        cerr << "scope-A: created previewer: \"" << result.uri() << "\"" << endl;
        return preview;
    }

private:
    string scope_id_;
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

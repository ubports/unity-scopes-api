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

#include <unity/scopes/CannedQuery.h>
#include <unity/scopes/CategorisedResult.h>
#include <unity/scopes/Category.h>
#include <unity/scopes/CategoryRenderer.h>
#include <unity/scopes/ScopeBase.h>
#include <unity/scopes/SearchReply.h>

#include <iostream>
#include <thread>

#define EXPORT __attribute__ ((visibility ("default")))

using namespace std;
using namespace unity::scopes;

// Example scope A: replies synchronously to a query. (Replies are returned before returning from the run() method.)

class MyQuery : public SearchQueryBase
{
public:
    MyQuery(string const& scope_id, CannedQuery const& query, CategoryRenderer const& renderer) :
        scope_id_(scope_id),
        query_(query),
        renderer_(renderer)
    {
        cerr << scope_id_ << ": query instance for \"" << query.query_string() << "\" created" << endl;
    }

    ~MyQuery()
    {
        cerr << scope_id_ << ": query instance for \"" << query_.query_string() << "\" destroyed" << endl;
    }

    virtual void cancelled() override
    {
        cerr << scope_id_ << ": \"" + query_.to_uri() + "\" cancelled" << endl;
    }

    virtual void run(SearchReplyProxy const& reply) override
    {
        if (!valid())
        {
            return;  // Query was cancelled
        }

        cerr << scope_id_ << ": run called for \"" << query_.query_string() << "\"" << endl;
        int const short_secs = 5;
        cerr << scope_id_ << ": sleeping for " << short_secs << " seconds" << endl;
        this_thread::sleep_for(chrono::seconds(short_secs));
        auto cat = reply->register_category("cat1", "Category 1", "", renderer_);
        CategorisedResult result(cat);
        result.set_uri("uri");
        result.set_title(scope_id_ + ": result 1 for query \"" + query_.query_string() + "\"");
        cerr << scope_id_ << ": pushing result" << endl;
        reply->push(result);
        int const long_secs = 50;
        cerr << scope_id_ << ": sleeping for " << long_secs << " seconds" << endl;
        this_thread::sleep_for(chrono::seconds(long_secs));
        cout << scope_id_ << ": query \"" << query_.query_string() << "\" complete" << endl;
    }

private:
    string scope_id_;
    CannedQuery query_;
    CategoryRenderer renderer_;
};

class MyScope : public ScopeBase
{
public:
    virtual int start(string const& scope_id, RegistryProxy const&) override
    {
        scope_id_ = scope_id;
        return VERSION;
    }

    virtual void stop() override {}

    virtual SearchQueryBase::UPtr search(CannedQuery const& q, SearchMetadata const& /* hints */) override
    {
        SearchQueryBase::UPtr query(new MyQuery(scope_id_, q, renderer_));
        cout << scope_id_ << ": created query: \"" << q.query_string() << "\"" << endl;
        return query;
    }

    virtual PreviewQueryBase::UPtr preview(Result const& result, ActionMetadata const&) override
    {
        cout << scope_id_ << ": preview: \"" << result.uri() << "\"" << endl;
        return nullptr;
    }

private:
    string scope_id_;
    CategoryRenderer renderer_;
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

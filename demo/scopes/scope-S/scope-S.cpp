/*
 * Copyright (C) 2013 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Michi Henning <michi.henning@canonical.com>
 */

#include <scopes/ScopeBase.h>
#include <scopes/Reply.h>
#include <scopes/ResultItem.h>
#include <scopes/Category.h>

#include <iostream>
#include <thread>

#define EXPORT __attribute__ ((visibility ("default")))

using namespace std;
using namespace unity::api::scopes;

// Example scope A: replies synchronously to a query. (Replies are returned before returning from the run() method.)

class MyQuery : public QueryBase
{
public:
    MyQuery(string const& query) :
        query_(query)
    {
        cerr << "MyQuery/" << query << " created" << endl;
    }

    ~MyQuery() noexcept
    {
        cerr << "MyQuery/" << query_ << " destroyed" << endl;
    }

    virtual void cancelled() override
    {
        cerr << "MyQuery/" << query_ << " cancelled" << endl;
    }

    virtual void run(ReplyProxy const& reply) override
    {
        cerr << "scope-slow: run called for \"" << query_ << "\"" << endl;
        this_thread::sleep_for(chrono::seconds(20));
        auto cat = reply->register_category("cat1", "Category 1", "", "{}");
        ResultItem result(cat);
        result.set_title("scope-slow: result 1 for query \"" + query_ + "\"");
        reply->push(result);
        cout << "scope-slow: query \"" << query_ << "\" complete" << endl;
    }

private:
    string query_;
};

class MyScope : public ScopeBase
{
public:
    virtual int start(string const&, RegistryProxy const&) override
    {
        return VERSION;
    }

    virtual void stop() override {}
    virtual void run() override {}

    virtual QueryBase::UPtr create_query(string const& q, VariantMap const& hints) override
    {
        QueryBase::UPtr query(new MyQuery(q));
        cout << "scope-slow: created query: \"" << q << "\"" << endl;

        auto it = hints.find("cardinality");
        if (it != hints.end())
        {
            cerr << "result cardinality: " << it->second.get_int() << endl;
        }

        it = hints.find("locale");
        if (it != hints.end())
        {
            cerr << "locale: " << it->second.get_string() << endl;
        }

        return query;
    }
};

extern "C"
{

    EXPORT
    unity::api::scopes::ScopeBase*
    // cppcheck-suppress unusedFunction
    UNITY_API_SCOPE_CREATE_FUNCTION()
    {
        return new MyScope;
    }

    EXPORT
    void
    // cppcheck-suppress unusedFunction
    UNITY_API_SCOPE_DESTROY_FUNCTION(unity::api::scopes::ScopeBase* scope_base)
    {
        delete scope_base;
    }

}

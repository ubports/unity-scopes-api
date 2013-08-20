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

#include <unity/api/scopes/ScopeBase.h>
#include <unity/api/scopes/Reply.h>

#include <iostream>

#define EXPORT __attribute__ ((visibility ("default")))

using namespace std;
using namespace unity::api::scopes;

// Example scope A: replies synchronously to a query. (Replies are returned before returning from the run() method.)

class MyQuery : public QueryBase
{
public:
    MyQuery(string const& scope_name, string const& query) :
        QueryBase(scope_name),
        query_(query)
    {
        cerr << "MyQuery/" << query << " created" << endl;
    }

    ~MyQuery() noexcept
    {
        cerr << "MyQuery/" << query_ << " destroyed" << endl;
    }

    virtual void cancelled(ReplyProxy const&) override
    {
    }

    virtual void run(ReplyProxy const& reply) override
    {
        reply->push("scope-A: result 1 for query \"" + query_ + "\"");
        cout << "scope-A: query \"" << query_ << "\" complete" << endl;
    }

private:
    string query_;
};

class MyScope : public ScopeBase
{
public:
    virtual int start(RegistryProxy const&) override
    {
        return VERSION;
    }

    virtual void stop() override {}
    virtual void run() override {}

    virtual QueryBase::UPtr create_query(string const& q) override
    {
        QueryBase::UPtr query(new MyQuery("scope-A", q));  // TODO: scope name should come from the run time
        cout << "scope-A: created query: \"" << q << "\"" << endl;
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

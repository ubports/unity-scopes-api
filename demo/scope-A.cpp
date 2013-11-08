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
#include <unity/api/scopes/Category.h>
#include <unity/api/scopes/ResultItem.h>

#include <iostream>

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
    }

    ~MyQuery() noexcept
    {
    }

    virtual void cancelled() override
    {
    }

    virtual void run(ReplyProxy const& reply) override
    {
        auto cat = std::shared_ptr<Category>(new Category("cat1"));
        ResultItem res(cat);
        res.set_title("scope-A: result 1 for query \"" + query_ + "\"");
        reply->push(res);
        cout << "scope-A: query \"" << query_ << "\" complete" << endl;
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

    virtual QueryBase::UPtr create_query(string const& q, VariantMap const&) override
    {
        QueryBase::UPtr query(new MyQuery(q));
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

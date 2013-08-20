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

#include <unity/api/scopes/Registry.h>
#include <unity/api/scopes/Reply.h>
#include <unity/api/scopes/ScopeBase.h>
#include <unity/UnityExceptions.h>

#include <iostream>

#define EXPORT __attribute__ ((visibility ("default")))

using namespace std;
using namespace unity::api::scopes;

// Example scope B: aggregates from scope C and D.

// A Reply instance remembers the query string and the reply object that was passed
// from upstream. Results from the child scopes are sent to that upstream reply object.

class SubReply : public ReplyBase
{
public:
    virtual bool push(string const& result) override
    {
        cout << "received result from " << scope_name_ << ": " << result << endl;
        return upstream_->push(result);
    }

    virtual void finished() override
    {
        cout << "query to " << scope_name_ << " complete" << endl;
    }

    SubReply(string const& scope_name, ReplyProxy const& upstream) :
        scope_name_(scope_name),
        upstream_(upstream)
    {
    }

private:
    string scope_name_;
    ReplyProxy upstream_;
};

class MyQuery : public QueryBase
{
public:
    MyQuery(string const& scope_name,
            string const& query,
            ScopeProxy const& scope_c,
            ScopeProxy const& scope_d) :
        scope_name_(scope_name),
        query_(query),
        scope_c_(scope_c),
        scope_d_(scope_d)
    {
    }

    virtual void cancelled(ReplyProxy const&)
    {
        cout << "query to " << scope_name_ << " was cancelled" << endl;
    }

    virtual void run(ReplyProxy const& upstream_reply)
    {
        ReplyBase::SPtr reply(new SubReply(scope_name_, upstream_reply));
        create_subquery(query_, scope_c_, reply);
        create_subquery(query_, scope_d_, reply);
    }

private:
    string scope_name_;
    string query_;
    ScopeProxy scope_c_;
    ScopeProxy scope_d_;
};

// MyScope aggregates from C and D.

class MyScope : public ScopeBase
{
public:
    virtual int start(string const& scope_name, RegistryProxy const& registry) override
    {
        scope_name_ = scope_name;

        // Lock up scopes C and D in the registry and remember their proxies.
        scope_c_ = registry->find("scope-C");
        scope_d_ = registry->find("scope-D");

        return VERSION;
    }

    virtual void stop() override {}

    virtual void run() override {}

    virtual QueryBase::UPtr create_query(string const& q) override
    {
        QueryBase::UPtr query(new MyQuery(scope_name_, q, scope_c_, scope_d_));
        cout << "scope-B: created query: \"" << q << "\"" << endl;
        return query;
    }

private:
    string scope_name_;
    ScopeProxy scope_c_;
    ScopeProxy scope_d_;
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

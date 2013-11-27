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

#include <scopes/Registry.h>
#include <scopes/Reply.h>
#include <scopes/Result.h>
#include <scopes/ScopeBase.h>
#include <unity/UnityExceptions.h>

#include <iostream>
#include <cassert>

#define EXPORT __attribute__ ((visibility ("default")))

using namespace std;
using namespace unity::api::scopes;

// Example scope B: aggregates from scope C and D.

// A Receiver instance remembers the query string and the reply object that was passed
// from upstream. Results from the child scopes are sent to that upstream reply object.

class Receiver: public ReceiverBase
{
public:
    virtual void push(Category::SCPtr category) override
    {
        cout << "received category: id=" << category->id() << endl;
    }

    virtual void push(Result result) override
    {
        cout << "received result from " << scope_name_ << ": " << result.uri() << ", " << result.title() << endl;
        try
        {
            upstream_->push(std::move(result));
        }
        catch (const unity::InvalidArgumentException &e)
        {
            cerr << "error pushing result: " << e.what() << endl;
        }
    }

    virtual void finished(Reason reason) override
    {
        cout << "query to " << scope_name_ << " complete, status: " << to_string(reason) << endl;
    }

    Receiver(string const& scope_name, ReplyProxy const& upstream) :
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

    virtual void cancelled()
    {
        cout << "query to " << scope_name_ << " was cancelled" << endl;
    }

    virtual void run(ReplyProxy const& upstream_reply)
    {
        // note, category id must mach categories received from scope C and D, otherwise result pushing will fail.
        try
        {
            upstream_reply->register_category("cat1", "title", "icon", "{}");
        }
        catch (const unity::InvalidArgumentException &e) // this shouldn't happen
        {
            cerr << "error registering category: " << e.what() << endl;
            assert(0);
        }

        ReceiverBase::SPtr reply(new Receiver(scope_name_, upstream_reply));
        create_subquery(scope_c_, query_, VariantMap(), reply);
        create_subquery(scope_d_, query_, VariantMap(), reply);
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

    virtual QueryBase::UPtr create_query(string const& q, VariantMap const&) override
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

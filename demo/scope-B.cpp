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
#include <unity/UnityExceptions.h>

#include <iostream>

#define EXPORT __attribute__ ((visibility ("default")))

using namespace std;
using namespace unity::api::scopes;

// Example scope B: aggregates from scope C and D.

// A Reply instance remembers the query string and the reply object that was passed
// from upstream. Results from the child scopes are sent to that upstream reply object.

class Reply : public ReplyBase
{
public:
    virtual void send(string const& result) override
    {
        cout << "received result from " << scope_name_ << ": " << result << endl;
        upstream_->send(result);
    }

    virtual void finished() override
    {
        cout << "query to " << scope_name_ << " complete" << endl;
    }

    Reply(string const& scope_name, ReplyProxy::SPtr const& upstream) :
        scope_name_(scope_name),
        upstream_(upstream)
    {
    }

private:
    string scope_name_;
    ReplyProxy::SPtr upstream_;
};

// MyScope agregates from C and D.

class MyScope : public ScopeBase
{
public:
    virtual int start(RegistryProxy::SPtr const& registry) override
    {
        // Lock up scopes C and D in the registry and remember their proxies.
        scope_c_ = registry->find("scope-C");
        if (!scope_c_)
        {
            throw unity::ResourceException("Cannot locate scope-C in registry");
        }
        scope_d_ = registry->find("scope-D");
        if (!scope_d_)
        {
            throw unity::ResourceException("Cannot locate scope-D in registry");
        }
        return VERSION;
    }

    virtual void stop() override {}
    virtual void run() override {}

    virtual void query(string const& q, ReplyProxy::SPtr const& reply) override
    {
        cout << "scope-B: received query string \"" << q << "\"" << endl;
        cout << "scope-B: forwarding query to scope-C and scope-D" << endl;

        shared_ptr<Reply> r;

        // Make a Reply instance and forward the query to C. C will deliver its
        // results to that Reply instance.
        r.reset(new Reply("scope-C", reply));
        scope_c_->query(q, r);

        // Do the same thing for D.
        r.reset(new Reply("scope-D", reply));
        scope_d_->query(q, r);

        // We are done. As the replies trickle in, the two reply instance (kept alive by our run time)
        // forward the results upstream.
        cout << "scope-B: query \"" << q << "\" complete" << endl;
    }

private:
    ScopeProxy::SPtr scope_c_;
    ScopeProxy::SPtr scope_d_;
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

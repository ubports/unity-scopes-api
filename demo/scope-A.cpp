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

#include <iostream>

#define EXPORT __attribute__ ((visibility ("default")))

using namespace std;
using namespace unity::api::scopes;

// Example scope A: replies synchronously to a query. (Replies are returned before returning from the query() method.)

class MyScope : public ScopeBase
{
public:
    virtual int start(RegistryProxy::SPtr const&) override { return VERSION; }
    virtual void stop() override {}
    virtual void run() override {}
    virtual void query(string const& q, ReplyProxy::SPtr const& reply) override
    {
        cout << "scope-A: received query string \"" << q << "\"" << endl;

        reply->send("scope-A: result 1 for query \"" + q + "\"");
        reply->finished();  // Optional

        cout << "scope-A: query \"" << q << "\" complete" << endl;
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

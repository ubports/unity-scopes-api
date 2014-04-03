/*
 * Copyright (C) 2014 Canonical Ltd
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

#include <unity/scopes/ScopeBase.h>

#include <iostream>
#include <thread>

#define EXPORT __attribute__ ((visibility ("default")))

using namespace std;
using namespace unity::scopes;

// Demonstration of building a click scope. This file has the bare minimum scope implementation.

class MyQuery : public SearchQueryBase
{
public:
    MyQuery()
    {
    }

    ~MyQuery()
    {
    }

    virtual void cancelled() override
    {
        cerr << "scope-click: received cancel request" << endl;
    }

    virtual void run(SearchReplyProxy const&) override
    {
        cerr << "scope-click: received query" << endl;
        this_thread::sleep_for(chrono::seconds(3));
        cerr << "scope-click: query complete" << endl;
    }
};

class MyScope : public ScopeBase
{
public:
    virtual int start(string const&, RegistryProxy const&) override
    {
        return VERSION;
    }

    virtual void stop() override {}

    virtual SearchQueryBase::UPtr search(CannedQuery const&, SearchMetadata const&) override
    {
        return SearchQueryBase::UPtr(new MyQuery);
    }

    virtual PreviewQueryBase::UPtr preview(Result const&, ActionMetadata const&) override
    {
        return nullptr;
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

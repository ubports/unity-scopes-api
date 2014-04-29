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

#include <unity/scopes/ScopeBase.h>

#include <iostream>
#include <thread>

#define EXPORT __attribute__ ((visibility ("default")))

using namespace std;
using namespace unity::scopes;

// Simplest possible scope: does absolutely nothing other than to implement the pure virtuals
// it inherits from its base classes. Despite this, the scope works correctly with a client.

class MyQuery : public SearchQueryBase
{
public:
    MyQuery(string const& scope_id)
        : scope_id_(scope_id)
    {
        cerr << scope_id_ << ": query instance created" << endl;
    }

    ~MyQuery()
    {
        cerr << scope_id_ << ": query instance destroyed" << endl;
    }

    virtual void cancelled() override
    {
        cerr << scope_id_ << ": received cancel request" << endl;
    }

    virtual void run(SearchReplyProxy const&) override
    {
        if (!valid())
        {
            return;  // Query was cancelled
        }

        cerr << scope_id_ << ": received query" << endl;
        this_thread::sleep_for(chrono::seconds(3));
        cerr << scope_id_ << ": query complete" << endl;
    }

private:
    string scope_id_;
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

    virtual SearchQueryBase::UPtr search(CannedQuery const&, SearchMetadata const&) override
    {
        return SearchQueryBase::UPtr(new MyQuery(scope_id_));
    }

    virtual PreviewQueryBase::UPtr preview(Result const&, ActionMetadata const&) override
    {
        return nullptr;
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

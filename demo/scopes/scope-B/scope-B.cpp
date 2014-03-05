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


// In this demo we include individual headers. You may also,
// if you prefer, include just the unity-scopes.h convenience
// header.
#include <unity/scopes/CategorisedResult.h>
#include <unity/scopes/CategoryRenderer.h>
#include <unity/scopes/SearchReply.h>
#include <unity/scopes/Registry.h>
#include <unity/scopes/ScopeBase.h>
#include <unity/scopes/ScopeExceptions.h>
#include <unity/scopes/SearchQueryBase.h>
#include <unity/UnityExceptions.h>

#include <cassert>
#include <iostream>

#define EXPORT __attribute__ ((visibility ("default")))

using namespace std;
using namespace unity::scopes;

// Example scope B: aggregates from scope C and D.

// A Receiver instance remembers the query string and the reply object that was passed
// from upstream. Results from the child scopes are sent to that upstream reply object.

class Receiver: public SearchListenerBase
{
public:
    virtual void push(Category::SCPtr category) override
    {
        cout << "received category: id=" << category->id() << endl;
    }

    virtual void push(CategorisedResult result) override
    {
        cout << "received result from " << scope_name_ << ": " << result.uri() << ", " << result.title() << endl;
        try
        {
            result.set_category(upstream_->lookup_category("catB"));
            upstream_->push(std::move(result));
        }
        catch (const unity::InvalidArgumentException &e)
        {
            cerr << "error pushing result: " << e.what() << endl;
        }
    }

    virtual void finished(Reason reason, string const& error_message) override
    {
        cout << "query to " << scope_name_ << " complete, status: " << to_string(reason);
        if (reason == ListenerBase::Error)
        {
            cout << ": " << error_message;
        }
        cout << endl;
    }

    Receiver(string const& scope_name, SearchReplyProxy const& upstream) :
        scope_name_(scope_name),
        upstream_(upstream)
    {
    }

private:
    string scope_name_;
    SearchReplyProxy upstream_;
};

class MyQuery : public SearchQueryBase
{
public:
    MyQuery(string const& scope_name,
            CannedQuery const& query,
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

    virtual void run(SearchReplyProxy const& upstream_reply)
    {
        // note, category id must mach categories received from scope C and D, otherwise result pushing will fail.
        try
        {
            CategoryRenderer rdr;
            upstream_reply->register_category("catB", "title", "icon", rdr);
        }
        catch (const unity::InvalidArgumentException &e) // this shouldn't happen
        {
            cerr << "error registering category: " << e.what() << endl;
            assert(0);
        }

        SearchListenerBase::SPtr reply(new Receiver(scope_name_, upstream_reply));
        subsearch(scope_c_, query_.query_string(), reply);
        subsearch(scope_d_, query_.query_string(), reply);
    }

private:
    string scope_name_;
    CannedQuery query_;
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

        if (!registry)
        {
            throw ConfigException(scope_name + ": No registry available, cannot locate child scopes");
        }
        // Lock up scopes C and D in the registry and remember their proxies.
        auto meta_c = registry->get_metadata("scope-C");
        scope_c_ = meta_c.proxy();
        auto meta_d = registry->get_metadata("scope-D");
        scope_d_ = meta_d.proxy();

        return VERSION;
    }

    virtual void stop() override {}

    virtual SearchQueryBase::UPtr search(CannedQuery const& q, SearchMetadata const&) override
    {
        SearchQueryBase::UPtr query(new MyQuery(scope_name_, q, scope_c_, scope_d_));
        cout << "scope-B: created query: \"" << q.query_string() << "\"" << endl;
        return query;
    }

    virtual PreviewQueryBase::UPtr preview(Result const& result, ActionMetadata const&) override
    {
        cout << "scope-B: preview: \"" << result.uri() << "\"" << endl;
        return nullptr;
    }

private:
    string scope_name_;
    ScopeProxy scope_c_;
    ScopeProxy scope_d_;
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

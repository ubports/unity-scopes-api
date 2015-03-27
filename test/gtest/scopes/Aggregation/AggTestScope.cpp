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

#include "AggTestScope.h"

#include <unity/scopes/CategorisedResult.h>
#include <unity/scopes/ScopeBase.h>
#include <unity/scopes/ScopeExceptions.h>
#include <unity/scopes/SearchReply.h>

#include <unity/UnityExceptions.h>
#include <unity/util/FileIO.h>

#include <mutex>
#include <sstream>
#include <thread>

using namespace std;
using namespace unity::scopes;

namespace
{

class Receiver: public SearchListenerBase
{
public:

    virtual void push(Category::SCPtr const& category) override
    {
        try
        {
            upstream_->register_category(category);
        }
        catch (unity::InvalidArgumentException const&)
        {
        }
    }

    virtual void push(CategorisedResult result) override
    {
        cerr << scope_id_ << ": received result: " << result.uri() << ", " << result.title() << endl;
        try
        {
            upstream_->push(std::move(result));
        }
        catch (const unity::InvalidArgumentException &e)
        {
            cerr << scope_id_ << ": error pushing result: " << e.what() << endl;
        }
    }

    virtual void finished(CompletionDetails const& details) override
    {
        cerr << scope_id_ << ": subquery complete, status: " << to_string(details.status())
             << ", msg: " << details.message() << endl;
    }

    Receiver(string const& scope_id, SearchReplyProxy const& upstream) :
        scope_id_(scope_id),
        upstream_(upstream)
    {
    }

private:
    string scope_id_;
    SearchReplyProxy upstream_;
};

class TestQuery : public SearchQueryBase
{
public:
    TestQuery(CannedQuery const& query,
              SearchMetadata const& metadata,
              string const& id,
              RegistryProxy const& reg)
        : SearchQueryBase(query, metadata)
        , id_(id)
        , registry_(reg)
    {
    }

    virtual void cancelled() override
    {
    }

    virtual void run(SearchReplyProxy const& reply) override
    {
        auto cat = reply->register_category(id_, "", "");
        SearchListenerBase::SPtr receiver(new Receiver(id_, reply));

        // Open the command file and, for each scope ID (one per line),
        // send a subquery to that scope.
        istringstream cmds(unity::util::read_text_file(string(TEST_RUNTIME_PATH) + "/" + id_ + ".cmd"));
        string child_id;
        while (getline(cmds, child_id))
        {
            if (child_id.empty())
            {
                continue;  // Ignore empty ID, which is what we get if we act as a leaf scope.
            }
            auto proxy = registry_->get_metadata(child_id).proxy();
            cerr << id_ << ": sending subsearch to " << child_id << endl;
            subsearch(proxy, id_ + "->" + child_id, receiver);
        }
        CategorisedResult res(cat);
        res.set_uri("uri");
        res.set_title(id_ + ": result for query \"" + query().query_string() + "\"");
        if (valid())
        {
            reply->push(res);
        }
    }

private:
    string id_;
    RegistryProxy registry_;
};

}  // namespace

void AggTestScope::start(string const& scope_id)
{
    lock_guard<mutex> lock(mutex_);
    id_ = scope_id;
}

void AggTestScope::stop()
{
}

void AggTestScope::run()
{
}

SearchQueryBase::UPtr AggTestScope::search(CannedQuery const& query, SearchMetadata const& metadata)
{
    lock_guard<mutex> lock(mutex_);
    return SearchQueryBase::UPtr(new TestQuery(query, metadata, id_, registry()));
}

PreviewQueryBase::UPtr AggTestScope::preview(Result const&, ActionMetadata const &)
{
    return nullptr;  // unused
}

ChildScopeList AggTestScope::find_child_scopes() const
{
    ChildScopeList list;
    list.emplace_back(ChildScope{"A", true});
    list.emplace_back(ChildScope{"B", true});
    list.emplace_back(ChildScope{"C", true});
    list.emplace_back(ChildScope{"D", true});
    return list;
}

extern "C"
{

    unity::scopes::ScopeBase*
    // cppcheck-suppress unusedFunction
    UNITY_SCOPE_CREATE_FUNCTION()
    {
        return new AggTestScope;
    }

    void
    // cppcheck-suppress unusedFunction
    UNITY_SCOPE_DESTROY_FUNCTION(unity::scopes::ScopeBase* scope_base)
    {
        delete scope_base;
    }
}

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
              ChildScopeList const& child_scopes)
        : SearchQueryBase(query, metadata)
        , id_(id)
        , child_scopes_(child_scopes)
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

            for (auto const& child_scope : child_scopes_)
            {
                if (child_scope.id == child_id && child_scope.enabled)
                {
                    cerr << id_ << ": sending subsearch to " << child_id << endl;
                    subsearch(child_scope, id_ + "->" + child_id, receiver);
                    break;
                }
            }
        }
        CategorisedResult res(cat);
        res.set_uri("uri");
        res.set_title(id_ + ": result for query \"" + query().query_string() + "\"");

        std::string aggregated_keywords;
        auto i = search_metadata().aggregated_keywords().size();
        for (auto const& keyword : search_metadata().aggregated_keywords())
        {
            aggregated_keywords += (--i == 0) ? keyword : keyword + ", ";
        }
        res["is_aggregated"] = search_metadata().is_aggregated();
        res["aggregated_keywords"] = aggregated_keywords;

        if (valid())
        {
            reply->push(res);
        }
    }

private:
    string id_;
    ChildScopeList child_scopes_;
};

}  // namespace

AggTestScope::AggTestScope()
    : metadata_("", "")
{
}

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
    metadata_ = metadata;
    return SearchQueryBase::UPtr(new TestQuery(query, metadata, id_, child_scopes()));
}

PreviewQueryBase::UPtr AggTestScope::preview(Result const&, ActionMetadata const &)
{
    return nullptr;  // unused
}

ChildScopeList AggTestScope::find_child_scopes() const
{
    ChildScopeList list;
    list.emplace_back(ChildScope{"A", registry()->get_metadata("A"), true, get_keywords("A")});
    list.emplace_back(ChildScope{"B", registry()->get_metadata("B"), true, get_keywords("B")});
    list.emplace_back(ChildScope{"C", registry()->get_metadata("C"), true, get_keywords("C")});
    list.emplace_back(ChildScope{"D", registry()->get_metadata("D"), true, get_keywords("D")});
    return list;
}

set<string> AggTestScope::get_keywords(string const& child_id) const
{
    set<string> keywords;
    if (metadata_.contains_hint(child_id) &&
        metadata_[child_id].which() == Variant::Type::Array)
    {
        for (auto const& kw : metadata_[child_id].get_array())
        {
            keywords.emplace(kw.get_string());
        }
    }
    return keywords;
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

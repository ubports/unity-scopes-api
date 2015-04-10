/*
 * Copyright (C) 2015 Canonical Ltd
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
#include <unity/scopes/SearchReply.h>

#include <unity/UnityExceptions.h>

using namespace std;
using namespace unity::scopes;

namespace
{

class AggregatorScope : public ScopeBase
{
public:
    virtual void start(std::string const&) override
    {
    }

    virtual void stop() override
    {
    }

    virtual void run() override
    {
    }

    virtual SearchQueryBase::UPtr search(CannedQuery const&, SearchMetadata const&) override;

    virtual PreviewQueryBase::UPtr preview(Result const&, ActionMetadata const&) override
    {
        return nullptr;  // unused
    }
};

class Receiver : public SearchListenerBase
{
public:
    virtual void push(Category::SCPtr const& category) override
    {
        try
        {
            upstream_->register_category(category);
            cerr << "agg: pushed cat" << endl;
        }
        catch (unity::InvalidArgumentException const&)
        {
        }
    }

    virtual void push(CategorisedResult result) override
    {
        upstream_->push(std::move(result));
    }

    virtual void finished(CompletionDetails const& /* details */) override
    {
        upstream_->finished();
    }

    Receiver(SearchReplyProxy const& upstream)
        : upstream_(upstream)
    {
    }

private:
    SearchReplyProxy upstream_;
};

class TestQuery : public SearchQueryBase
{
public:
    TestQuery(CannedQuery const& query, SearchMetadata const& metadata, ScopeProxy const& leaf_proxy)
        : SearchQueryBase(query, metadata)
        , leaf_proxy_(leaf_proxy)
    {
    }

    virtual void cancelled() override
    {
    }

    virtual void run(SearchReplyProxy const& reply) override
    {
        auto cat = reply->register_category("cat", "", "");
        SearchListenerBase::SPtr receiver(new Receiver(reply));
        subsearch(leaf_proxy_, query().query_string(), receiver);
    }

private:
    ScopeProxy leaf_proxy_;
};

}  // namespace

SearchQueryBase::UPtr AggregatorScope::search(CannedQuery const& query, SearchMetadata const& metadata)
{
    auto leaf_proxy = registry()->get_metadata("Leaf").proxy();
    return SearchQueryBase::UPtr(new TestQuery(query, metadata, leaf_proxy));
}

extern "C" {

ScopeBase*
// cppcheck-suppress unusedFunction
UNITY_SCOPE_CREATE_FUNCTION()
{
    return new AggregatorScope;
}

void
// cppcheck-suppress unusedFunction
UNITY_SCOPE_DESTROY_FUNCTION(ScopeBase* scope_base)
{
    delete scope_base;
}
}

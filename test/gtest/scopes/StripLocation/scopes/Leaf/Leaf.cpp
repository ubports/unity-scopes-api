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

using namespace std;
using namespace unity::scopes;

namespace
{

class LeafScope : public unity::scopes::ScopeBase
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

    virtual unity::scopes::SearchQueryBase::UPtr search(unity::scopes::CannedQuery const&,
                                                        unity::scopes::SearchMetadata const&) override;

    virtual unity::scopes::PreviewQueryBase::UPtr preview(unity::scopes::Result const&,
                                                          unity::scopes::ActionMetadata const&) override
    {
        return nullptr;  // unused
    }
};

class TestQuery : public SearchQueryBase
{
public:
    TestQuery(CannedQuery const& query, SearchMetadata const& metadata)
        : SearchQueryBase(query, metadata)
    {
    }

    virtual void cancelled() override
    {
    }

    virtual void run(SearchReplyProxy const& reply) override
    {
        auto cat = reply->register_category("cat", "", "");
        CategorisedResult res(cat);
        res.set_uri("uri");
        res["location_data"] = search_metadata().has_location();
        reply->push(res);
    }
};

}  // namespace

SearchQueryBase::UPtr LeafScope::search(CannedQuery const& query, SearchMetadata const& metadata)
{
    return SearchQueryBase::UPtr(new TestQuery(query, metadata));
}

extern "C" {

unity::scopes::ScopeBase*
// cppcheck-suppress unusedFunction
UNITY_SCOPE_CREATE_FUNCTION()
{
    return new LeafScope;
}

void
// cppcheck-suppress unusedFunction
UNITY_SCOPE_DESTROY_FUNCTION(unity::scopes::ScopeBase* scope_base)
{
    delete scope_base;
}

}

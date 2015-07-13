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

#include "CacheScope.h"

#include <unity/scopes/CategorisedResult.h>
#include <unity/scopes/OptionSelectorFilter.h>
#include <unity/scopes/ScopeBase.h>
#include <unity/scopes/ScopeExceptions.h>
#include <unity/scopes/SearchReply.h>

#include <boost/filesystem.hpp>
#include <unity/UnityExceptions.h>
#include <unity/util/FileIO.h>

#include <atomic>
#include <mutex>
#include <cstdint>

using namespace std;
using namespace unity::scopes;

namespace
{

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
        if (id_ == "AlwaysPushFromCacheScope")
        {
            // So we get coverage on pushing from cache before a cache file exists.
            reply->push_surfacing_results_from_cache();
            return;
        }

        if (query().query_string().empty())
        {
            // If there is a cache file, we use it to push.
            boost::system::error_code ec;
            if (boost::filesystem::exists(TEST_RUNTIME_PATH "/unconfined/CacheScope/.surfacing_cache", ec))
            {
                reply->push_surfacing_results_from_cache();
                // We do this twice, to get coverage on the double-call guard.
                reply->push_surfacing_results_from_cache();
                return;
            }
        }
        else if (query().query_string() == "non-empty from cache")
        {
            // This gives us coverage for the case that we are pushing from the cache for a non-empty query.
            reply->push_surfacing_results_from_cache();
            return;
        }

        auto depts = Department::create("", query(), "Top");
        auto sub_dept = Department::create("sub" + query().query_string(), query(), "Sub");
        depts->add_subdepartment(move(sub_dept));
        reply->register_departments(move(depts));

        Filters filters;
        OptionSelectorFilter::SPtr filter = OptionSelectorFilter::create("f1", "Choose an option", false);
        filter->add_option("o1", "Option 1");
        auto active_opt = filter->add_option("o2", "Option 2");
        FilterState fstate;
        filter->update_state(fstate, active_opt, true);
        filters.push_back(filter);
        if (valid())
        {
            reply->push(filters, fstate);
        }

        auto cat = reply->register_category(id_, "", "");
        CategorisedResult res(cat);
        res.set_uri("uri");
        res.set_title(query().query_string());
        int64_t v = 1;
        res["int64value"] = Variant(v);
        int64_t v2 = INT64_MAX;
        res["int64value2"] = Variant(v2);
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

void CacheScope::start(string const& scope_id)
{
    lock_guard<mutex> lock(mutex_);
    id_ = scope_id;
}

void CacheScope::stop()
{
}

void CacheScope::run()
{
}

SearchQueryBase::UPtr CacheScope::search(CannedQuery const& query, SearchMetadata const& metadata)
{
    lock_guard<mutex> lock(mutex_);
    return SearchQueryBase::UPtr(new TestQuery(query, metadata, id_, registry()));
}

PreviewQueryBase::UPtr CacheScope::preview(Result const&, ActionMetadata const &)
{
    return nullptr;  // unused
}

extern "C"
{

    unity::scopes::ScopeBase*
    // cppcheck-suppress unusedFunction
    UNITY_SCOPE_CREATE_FUNCTION()
    {
        return new CacheScope;
    }

    void
    // cppcheck-suppress unusedFunction
    UNITY_SCOPE_DESTROY_FUNCTION(unity::scopes::ScopeBase* scope_base)
    {
        delete scope_base;
    }
}

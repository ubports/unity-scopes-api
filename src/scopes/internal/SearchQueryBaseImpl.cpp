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
 *              Pawel Stolowski <pawel.stolowski@canonical.com>
 */

#include <unity/scopes/internal/SearchQueryBaseImpl.h>

#include <unity/scopes/internal/DfltConfig.h>
#include <unity/scopes/internal/QueryCtrlImpl.h>
#include <unity/scopes/internal/RuntimeImpl.h>
#include <unity/scopes/internal/ScopeImpl.h>
#include <unity/scopes/internal/SearchMetadataImpl.h>
#include <unity/scopes/ScopeExceptions.h>

#include <boost/filesystem/operations.hpp>
#include <unity/UnityExceptions.h>
#include <unity/util/IniParser.h>

#include <algorithm>
#include <cassert>

namespace unity
{

namespace scopes
{

namespace internal
{

using namespace std;

SearchQueryBaseImpl::SearchQueryBaseImpl(CannedQuery const& query, SearchMetadata const& metadata)
    : QueryBaseImpl(),
      canned_query_(query),
      search_metadata_(metadata),
      valid_(true)
{
}

CannedQuery SearchQueryBaseImpl::query() const
{
    return canned_query_;
}

SearchMetadata SearchQueryBaseImpl::search_metadata() const
{
    return search_metadata_;
}

QueryCtrlProxy SearchQueryBaseImpl::subsearch(ScopeProxy const& scope,
                                              std::string const& query_string,
                                              std::string const& department_id,
                                              FilterState const& filter_state,
                                              std::unique_ptr<Variant> user_data,
                                              SearchMetadata const& metadata,
                                              SearchListenerBase::SPtr const& reply)
{
    if (!scope)
    {
        throw InvalidArgumentException("QueryBase::subsearch(): scope cannot be nullptr");
    }
    if (!reply)
    {
        throw InvalidArgumentException("QueryBase::subsearch(): reply cannot be nullptr");
    }

    auto query_ctrl = check_for_query_loop(scope, reply);
    if (query_ctrl)
    {
        return query_ctrl;  // Loop was detected, return dummy QueryCtrlProxy.
    }

    // scope_impl can be nullptr if we use a mock scope: TypedScopeFixture<testing::Scope>.
    // If so, we call the normal search without passing the history through because
    // we don't need loop detection for mock scopes.
    auto scope_impl = dynamic_pointer_cast<ScopeImpl>(scope);
    if (scope_impl)
    {
        // TODO: HACK: Strip location info from metadata if the child isn't allowed to see it.
        //             We need to remove this once a scope is able to do this itself.
        SearchMetadata clean_metadata = filter_metadata(scope_impl, metadata);

        query_ctrl = scope_impl->search(query_string, department_id, filter_state, std::move(user_data), clean_metadata,
                                        history_, reply);
    }
    else
    {
        query_ctrl = user_data ?
                         scope->search(query_string, department_id, filter_state, *user_data, metadata, reply) :
                         scope->search(query_string, department_id, filter_state, metadata, reply);
    }

    lock_guard<mutex> lock(mutex_);
    subqueries_.push_back(query_ctrl);  // Remember subsearch in case we get a cancel() later that we need to forward.
    return query_ctrl;
}

void SearchQueryBaseImpl::cancel()
{
    lock_guard<mutex> lock(mutex_);

    if (!valid_)
    {
        return;
    }
    valid_ = false;
    for (auto& ctrl : subqueries_)
    {
        ctrl->cancel(); // Forward the cancellation to any subqueries that might be active
    }
    // We release the memory for the subquery controls here. That's just a micro-optimization
    // because this QueryBase will be destroyed shortly anyway, once the cancelled() (and possibly
    // run()) methods of the application return. (Not deallocating here would work too.)
    vector<QueryCtrlProxy>().swap(subqueries_);
}

void SearchQueryBaseImpl::set_department_id(std::string const& department_id)
{
    lock_guard<mutex> lock(mutex_);
    department_id_ = department_id;
}

std::string SearchQueryBaseImpl::department_id() const
{
    lock_guard<mutex> lock(mutex_);
    return department_id_;
}

void SearchQueryBaseImpl::set_client_id(std::string const& id)
{
    lock_guard<mutex> lock(mutex_);
    client_id_ = id;
}

void SearchQueryBaseImpl::set_history(History const& h)
{
    lock_guard<mutex> lock(mutex_);
    history_ = h;
}

bool SearchQueryBaseImpl::valid() const
{
    lock_guard<mutex> lock(mutex_);
    return valid_;
}

// Check if this query has been through this aggregator before by checking the
// history. The history is a list of <client, aggregator, receiver> scope ID tuples.
// If a query loops around to this aggregator, and has been sent to the same child
// previously on behalf of the same client, we immediately call finished to indicate
// an empty result set. If a loop is detected, we return a dummy QueryCtrl. Otherwise,
// we add the current <client, aggregator, receiver> tuple to the history before forwarding the search.
//
// Note that we do this unconditionally, whether the receiver is a leaf or an aggregator.
// This means that an aggregator can ask a *leaf scope* more than once for data.
// This works because normal searches do not check the query history (only
// subsearches do). But an aggregator cannot ask another *aggregator* more
// than once for data, even if that would not create a loop.
//
// It is possible for an aggregator to send a query to itself, but only once.

QueryCtrlProxy SearchQueryBaseImpl::check_for_query_loop(ScopeProxy const& scope,
                                                         SearchListenerBase::SPtr const& reply)
{
    shared_ptr<QueryCtrlImpl> ctrl_proxy;

    lock_guard<mutex> lock(mutex_);

    HistoryData tuple = make_tuple(client_id_, canned_query_.scope_id(), scope->identity());
    auto it = find(history_.begin(), history_.end(), tuple);
    if (it != history_.end())
    {
        // Query has been here before from the same client and with the same child scope as the target.
        reply->finished(CompletionDetails(CompletionDetails::OK,
                                          "empty result set due to aggregator loop or repeated query on aggregating scope "
                                          + get<1>(tuple)));
        auto scope_impl = dynamic_pointer_cast<ScopeImpl>(scope);
        auto logger = scope_impl->runtime()->logger();
        ctrl_proxy = make_shared<QueryCtrlImpl>(nullptr, nullptr);  // Dummy proxy in already-cancelled state
        BOOST_LOG_SEV(logger, Logger::Warning)
            << "query loop for query \"" << canned_query_.query_string()
            << "\", client: " << get<0>(tuple)
            << ", aggregator: " << get<1>(tuple)
            << ", receiver: " << get<2>(tuple) << endl;
    }
    else
    {
        history_.push_back(tuple);
    }

    return ctrl_proxy;  // null proxy if there was no loop
}

namespace
{

// Return a copy of the passed metadata sans its location information.

SearchMetadata strip_location(SearchMetadata const& metadata)
{
    SearchMetadata stripped(metadata);
    stripped.remove_location();
    return stripped;
}

} // namespace

// Return a copy of the metadata without location information if the scope isn't alloweed
// to get location data. Otherwise, return the metadata unchanged.

SearchMetadata SearchQueryBaseImpl::filter_metadata(shared_ptr<ScopeImpl>const& scope, SearchMetadata const& metadata)
{
    assert(scope);

    namespace fs = boost::filesystem;

    if (!metadata.has_location())
    {
        return metadata;  // Nothing to strip in the first place.
    }

    static string config_dir(scope->runtime()->config_directory());

    // We cache the write time and the setting for each scope,
    // so we re-parse the settings file only if it has changed.
    struct LocationPerm
    {
        time_t last_write_time;
        bool   perm;
    };
    static map<string, LocationPerm> perms;  // Key is scope ID
    static mutex perms_mutex;

    string const scope_id = scope->identity();
    fs::path settings_path = config_dir + "/" + scope_id + "/settings.ini";

    bool permitted = DFLT_LOCATION_PERMITTED;

    lock_guard<mutex> lock(perms_mutex);

    auto it = perms.find(scope_id);
    try
    {
        // Check cache.
        if (it != perms.end())
        {
            if (fs::last_write_time(settings_path) == it->second.last_write_time)
            {
                // Got cache hit, and file write time is still the same.
                return it->second.perm ? metadata : strip_location(metadata);
            }
        }

        // We don't have cached data yet, or the file has been updated.
        unity::util::IniParser parser(settings_path.native().c_str());
        permitted = parser.get_boolean("General", "internal.location");

        // Update cache.
        perms.insert(make_pair(scope_id, LocationPerm{ fs::last_write_time(settings_path), permitted }));
    }
    catch (std::exception const&)
    {
        // Couldn't stat or parse the settings file, so we return the default.
    }

    return permitted ? metadata : strip_location(metadata);
}

} // namespace internal

} // namespace scopes

} // namespace unity

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
 * Authored by: Marcus Tomlinson <marcus.tomlinson@canonical.com>
 */

#include <unity/scopes/internal/smartscopes/SSRegistryObject.h>

#include <unity/scopes/internal/JsonCppNode.h>
#include <unity/scopes/internal/JsonSettingsSchema.h>
#include <unity/scopes/internal/RegistryException.h>
#include <unity/scopes/internal/RuntimeConfig.h>
#include <unity/scopes/internal/RuntimeImpl.h>
#include <unity/scopes/internal/ScopeImpl.h>
#include <unity/scopes/internal/ScopeMetadataImpl.h>
#include <unity/scopes/internal/smartscopes/HttpClientNetCpp.h>
#include <unity/scopes/internal/Utils.h>
#include <unity/scopes/ScopeExceptions.h>
#include <unity/UnityExceptions.h>

namespace unity
{

namespace scopes
{

namespace internal
{

namespace smartscopes
{

SSRegistryObject::SSRegistryObject(MiddlewareBase::SPtr middleware,
                                   SSConfig const& ss_config,
                                   std::string const& ss_scope_endpoint,
                                   std::string const& sss_url,
                                   bool caching_enabled)
    : refresh_stopped_(false)
    , middleware_(middleware)
    , ss_scope_endpoint_(ss_scope_endpoint)
    , regular_refresh_timeout_(ss_config.reg_refresh_rate())
    , next_refresh_timeout_(ss_config.reg_refresh_rate())
    , failed_refresh_timeout_(ss_config.reg_refresh_fail_timeout())
    , caching_enabled_(caching_enabled)
{
    // get locale_ from LANGUAGE environment variable (if not available, the default is "").
    char const* locale = getenv("LANGUAGE");
    if (locale && *locale != '\0')
    {
        // LANGUAGE is a colon separated list of locales in order of priority.
        // We are only concerned with the highest priority locale, so we split it out here.
        locale_ = std::string(locale);
        size_t colon_pos = locale_.find(':');
        if (colon_pos != std::string::npos)
        {
            locale_.resize(colon_pos);
        }
    }

    // get remote scopes then start the auto-refresh background thread
    try
    {
	//TODO:Clean removal of smartscopes. Currently just a quick hack to get rid of network queries
        //get_remote_scopes();
    }
    catch (std::exception const& e)
    {

        middleware_->runtime()->logger()() << "SSRegistryObject: get_remote_scopes() failed: " << e.what();
    }

    refresh_thread_ = std::thread(&SSRegistryObject::refresh_thread, this);

    if (middleware)
    {
        try
        {
            publisher_ = middleware->create_publisher(middleware->runtime()->ss_registry_identity());
        }
        catch (std::exception const& e)
        {
            middleware_->runtime()->logger()() << "SSRegistryObject(): failed to create registry publisher: "
                                               << e.what();
        }
    }
}

SSRegistryObject::~SSRegistryObject()
{
    // stop the refresh thread
    {
        std::lock_guard<std::mutex> lock(refresh_mutex_);

        refresh_stopped_ = true;
        refresh_cond_.notify_all();
    }

    refresh_thread_.join();
}

ScopeMetadata SSRegistryObject::get_metadata(std::string const& scope_id) const
{
    // If the id is empty, it was sent as empty by the remote client.
    if (scope_id.empty())
    {
        throw unity::InvalidArgumentException("SSRegistryObject: Cannot search for scope with empty id");
    }

    std::lock_guard<std::mutex> lock(scopes_mutex_);

    auto const& it = scopes_.find(scope_id);
    if (it == scopes_.end())
    {
        throw NotFoundException("SSRegistryObject::get_metadata(): no such scope", scope_id);
    }
    return it->second;
}

MetadataMap SSRegistryObject::list() const
{
    std::lock_guard<std::mutex> lock(scopes_mutex_);
    return scopes_;
}

ObjectProxy SSRegistryObject::locate(std::string const& identity)
{
    // Smart Scopes are not fork and execed, so we simply return the proxy here
    return get_metadata(identity).proxy();
}

bool SSRegistryObject::is_scope_running(std::string const&)
{
    throw internal::RegistryException("SSRegistryObject::is_scope_running(): operation not available");
}

bool SSRegistryObject::has_scope(std::string const& scope_id) const
{
    std::lock_guard<std::mutex> lock(scopes_mutex_);

    auto const& it = scopes_.find(scope_id);
    if (it == scopes_.end())
    {
        return false;
    }
    return true;
}

std::string SSRegistryObject::get_base_url(std::string const& scope_id) const
{
    std::lock_guard<std::mutex> lock(scopes_mutex_);

    auto base_url = base_urls_.find(scope_id);
    if (base_url != end(base_urls_))
    {
        return base_url->second;
    }
    else
    {
        throw NotFoundException("SSRegistryObject::get_base_url(): no such scope", scope_id);
    }
}

SettingsDB::SPtr SSRegistryObject::get_settings_db(std::string const& scope_id) const
{
    std::lock_guard<std::mutex> lock(scopes_mutex_);

    auto settings_db = settings_defs_.find(scope_id);
    if (settings_db != settings_defs_.end())
    {
        return settings_db->second.db;
    }
    return nullptr;
}

void SSRegistryObject::refresh_thread()
{
    std::lock_guard<std::mutex> lock(refresh_mutex_);

    while (!refresh_stopped_)
    {
        refresh_cond_.wait_for(refresh_mutex_, std::chrono::seconds(next_refresh_timeout_));

        if (!refresh_stopped_)
        {
            try
            {
		//TODO: Clean removal of smartscopes. Currently just a quick hack to get rid of network queries
                //get_remote_scopes();
            }
            catch (std::exception const& e)
            {
                middleware_->runtime()->logger()() << "SSRegistryObject: get_remote_scopes() failed: " << e.what();
            }
        }
    }
}

// Must be called with refresh_mutex_ locked
void SSRegistryObject::get_remote_scopes()
{
}

bool SSRegistryObject::add(RemoteScope const& remotedata, ScopeMetadata const& metadata, MetadataMap& scopes, std::map<std::string, std::string>& urls)
{
    if (metadata.scope_id().empty())
    {
        throw unity::InvalidArgumentException("SSRegistryObject: Cannot add scope with empty id");
    }

    // store the base url under a scope ID key
    urls[metadata.scope_id()] = remotedata.base_url;

    // store the scope metadata in scopes_
    auto const& pair = scopes.insert(make_pair(metadata.scope_id(), metadata));
    if (!pair.second)
    {
        // Replace already existing entry with this one
        scopes.erase(pair.first);
        scopes.insert(make_pair(metadata.scope_id(), metadata));
        return false;
    }
    return true;
}

}  // namespace smartscopes

}  // namespace internal

}  // namespace scopes

}  // namespace unity

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

#include <unity/scopes/internal/smartscopes/HttpClientQt.h>
#include <unity/scopes/internal/smartscopes/JsonCppNode.h>

#include <unity/scopes/ScopeExceptions.h>
#include <unity/scopes/internal/ScopeMetadataImpl.h>
#include <unity/UnityExceptions.h>

namespace unity
{

namespace scopes
{

namespace internal
{

namespace smartscopes
{

SSRegistryObject::SSRegistryObject()
    : ssclient_(std::make_shared< HttpClientQt >(4),
                std::make_shared< JsonCppNode >()),
      refresh_thread_(std::thread(&SSRegistryObject::refresh_thread, this)),
      refresh_stopped_(false)
{
}

SSRegistryObject::~SSRegistryObject() noexcept
{
    {
        std::lock_guard<std::mutex> lock(refresh_mutex_);

        refresh_stopped_ = true;
        refresh_wait_.notify_all();
    }

    refresh_thread_.join();
}

ScopeMetadata SSRegistryObject::get_metadata(std::string const& scope_name)
{
    // If the name is empty, it was sent as empty by the remote client.
    if (scope_name.empty())
    {
        throw unity::InvalidArgumentException("Registry: Cannot search for scope with empty name");
    }

    std::lock_guard<std::mutex> lock(scopes_mutex_);

    auto const& it = scopes_.find(scope_name);
    if (it == scopes_.end())
    {
        throw NotFoundException("Registry::get_metadata(): no such scope",  scope_name);
    }
    return it->second;
}

MetadataMap SSRegistryObject::list()
{
    std::lock_guard<std::mutex> lock(scopes_mutex_);
    return scopes_;
}

ScopeProxy SSRegistryObject::locate(std::string const& scope_name)
{
    return ScopeProxy();
}

void SSRegistryObject::refresh_thread()
{
    std::lock_guard<std::mutex> lock(refresh_mutex_);

    while (!refresh_stopped_)
    {
        get_remote_scopes();

        refresh_wait_.wait_for(refresh_mutex_, std::chrono::hours(24));
    }
}

void SSRegistryObject::get_remote_scopes()
{
    std::vector<RemoteScope> remote_scopes = ssclient_.get_remote_scopes();

    std::lock_guard<std::mutex> lock(scopes_mutex_);

    for( RemoteScope& scope : remote_scopes )
    {
        std::unique_ptr<ScopeMetadataImpl> mi(new ScopeMetadataImpl(nullptr));

        mi->set_scope_name(scope.name);
        mi->set_display_name(scope.name);
        mi->set_description(scope.description);
        mi->set_art("");
        mi->set_icon("");
        mi->set_search_hint("");
        mi->set_hot_key("");

        mi->set_proxy(ScopeProxy()); ///! given to me from usa (pass ssclient* to proxy)

        ///!auto meta = ScopeMetadataImpl::create(move(mi));
        ///!add(scope.name, move(meta));
    }
}

bool SSRegistryObject::add(std::string const& scope_name, ScopeMetadata const& metadata)
{
    if (scope_name.empty())
    {
        throw unity::InvalidArgumentException("Registry: Cannot add scope with empty name");
    }

    std::lock_guard<std::mutex> lock(scopes_mutex_);

    auto const& pair = scopes_.insert(make_pair(scope_name, metadata));
    if (!pair.second)
    {
        // Replace already existing entry with this one
        scopes_.erase(pair.first);
        scopes_.insert(make_pair(scope_name, metadata));
        return false;
    }
    return true;
}

} // namespace smartscopes

} // namespace internal

} // namespace scopes

} // namespace unity

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

#include <unity/scopes/internal/smartscopes/SmartRegistryObject.h>

#include <unity/scopes/internal/smartscopes/HttpClientQt.h>
#include <unity/scopes/internal/smartscopes/JsonCppNode.h>

#include <unity/scopes/ScopeExceptions.h>
#include <unity/scopes/internal/ScopeMetadataImpl.h>
#include <unity/UnityExceptions.h>

using namespace std;
using namespace unity::scopes::internal::smartscopes;

namespace unity
{

namespace scopes
{

namespace internal
{

namespace smartscopes
{

SmartRegistryObject::SmartRegistryObject()
    : ssclient_(make_shared< HttpClientQt >(4),
                make_shared< JsonCppNode >())
{
    get_remote_scopes();
}

SmartRegistryObject::~SmartRegistryObject() noexcept
{
}

ScopeMetadata SmartRegistryObject::get_metadata(std::string const& scope_name)
{
    // If the name is empty, it was sent as empty by the remote client.
    if (scope_name.empty())
    {
        throw unity::InvalidArgumentException("Registry: Cannot search for scope with empty name");
    }

    lock_guard<decltype(mutex_)> lock(mutex_);

    auto const& it = scopes_.find(scope_name);
    if (it == scopes_.end())
    {
        throw NotFoundException("Registry::get_metadata(): no such scope",  scope_name);
    }
    return it->second;
}

MetadataMap SmartRegistryObject::list()
{
    lock_guard<decltype(mutex_)> lock(mutex_);
    return scopes_;
}

void SmartRegistryObject::get_remote_scopes()
{
    std::vector<RemoteScope> remote_scopes = ssclient_.get_remote_scopes();

    for( RemoteScope& scope : remote_scopes )
    {
        unique_ptr<ScopeMetadataImpl> mi(new ScopeMetadataImpl(nullptr));

        mi->set_scope_name(scope.name);
        mi->set_proxy(ScopeProxy()); ///! given to me from usa (pass ssclient* to proxy)
        mi->set_display_name(scope.name);
        mi->set_description(scope.description);
        //mi->set_art("");
        //mi->set_icon("");
        //mi->set_search_hint("");
        //mi->set_hot_key("");

        auto meta = ScopeMetadataImpl::create(move(mi));
        add(scope.name, move(meta));
    }
}

bool SmartRegistryObject::add(std::string const& scope_name, ScopeMetadata const& metadata)
{
    if (scope_name.empty())
    {
        throw unity::InvalidArgumentException("Registry: Cannot add scope with empty name");
    }

    lock_guard<decltype(mutex_)> lock(mutex_);

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

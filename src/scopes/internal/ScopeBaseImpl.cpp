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
 * Authored by: Pawel Stolowski <pawel.stolowski@canonical.com>
 */

#include <unity/scopes/internal/ScopeBaseImpl.h>

#include <unity/UnityExceptions.h>
#include <unity/scopes/internal/SettingsDB.h>
#include <unity/scopes/Registry.h>
#include <unity/scopes/ScopeExceptions.h>

using namespace unity;
using namespace unity::scopes;
using namespace std;

namespace unity
{

namespace scopes
{

namespace internal
{

ScopeBaseImpl::ScopeBaseImpl()
    : scope_dir_initialized_(false)
    , cache_dir_initialized_(false)
    , app_dir_initialized_(false)
    , registry_initialized_(false)
    , settings_db_initialized_(false)
{
}

void ScopeBaseImpl::set_scope_directory(std::string const& path)
{
    lock_guard<mutex> lock(mutex_);
    scope_directory_ = path;
    scope_dir_initialized_ = true;
}

std::string ScopeBaseImpl::scope_directory() const
{
    lock_guard<mutex> lock(mutex_);
    if (!scope_dir_initialized_)
    {
        throw LogicException("ScopeBase::scope_directory() cannot be called from constructor");
    }
    // scope_dir_ is never empty, except for tests, so we don't throw if it is empty here.
    return scope_directory_;
}

void ScopeBaseImpl::set_cache_directory(std::string const& path)
{
    lock_guard<mutex> lock(mutex_);
    cache_directory_ = path;
    cache_dir_initialized_ = true;
}

std::string ScopeBaseImpl::cache_directory() const
{
    lock_guard<mutex> lock(mutex_);
    if (!cache_dir_initialized_)
    {
        throw LogicException("ScopeBase::cache_directory() cannot be called from constructor");
    }
    if (cache_directory_.empty())
    {
        throw ConfigException("ScopeBase::cache_directory(): no cache directory available");
    }
    return cache_directory_;
}

void ScopeBaseImpl::set_app_directory(std::string const& path)
{
    lock_guard<mutex> lock(mutex_);
    app_directory_ = path;
    app_dir_initialized_ = true;
}

std::string ScopeBaseImpl::app_directory() const
{
    lock_guard<mutex> lock(mutex_);
    if (!app_dir_initialized_)
    {
        throw LogicException("ScopeBase::app_directory() cannot be called from constructor");
    }
    if (app_directory_.empty())
    {
        throw ConfigException("ScopeBase::app_directory(): no app directory available");
    }
    return app_directory_;
}

void ScopeBaseImpl::set_tmp_directory(std::string const& path)
{
    lock_guard<mutex> lock(mutex_);
    tmp_directory_ = path;
    tmp_dir_initialized_ = true;
}

std::string ScopeBaseImpl::tmp_directory() const
{
    lock_guard<mutex> lock(mutex_);
    if (!tmp_dir_initialized_)
    {
        throw LogicException("ScopeBase::tmp_directory() cannot be called from constructor");
    }
    if (tmp_directory_.empty())
    {
        throw ConfigException("ScopeBase::tmp_directory(): no tmp directory available");
    }
    return tmp_directory_;
}

void ScopeBaseImpl::set_registry(RegistryProxy const& registry)
{
    lock_guard<mutex> lock(mutex_);
    registry_ = registry;
    registry_initialized_ = true;
}

RegistryProxy ScopeBaseImpl::registry() const
{
    lock_guard<mutex> lock(mutex_);
    if (!registry_initialized_)
    {
        throw LogicException("ScopeBase::registry() cannot be called from constructor");
    }
    // registry_ can be a null proxy.
    return registry_;
}

SettingsDB::SPtr ScopeBaseImpl::settings_db() const
{
    lock_guard<mutex> lock(mutex_);
    if (!settings_db_initialized_)
    {
        throw LogicException("ScopeBase::settings_db() cannot be called from constructor");
    }
    return db_;
}

void ScopeBaseImpl::set_settings_db(SettingsDB::SPtr const& db)
{
    lock_guard<mutex> lock(mutex_);
    db_ = db;
    settings_db_initialized_ = true;
}

VariantMap ScopeBaseImpl::settings() const
{
    lock_guard<mutex> lock(mutex_);
    if (!settings_db_initialized_)
    {
        throw LogicException("ScopeBase::settings_db() cannot be called from constructor");
    }
    return db_ ? db_->settings() : VariantMap();
}

void ScopeBaseImpl::set_config_directory(std::string const& path)
{
    lock_guard<mutex> lock(mutex_);
    config_directory_ = path;
}

ChildScopeList ScopeBaseImpl::child_scopes() const
{
    // The default behaviour of this method is to simply return all available scopes on the system.
    // This translates to: Any scope may potentially be aggregated by this scope.
    lock_guard<mutex> lock(mutex_);
    ChildScopeList return_list;
    auto all_scopes = registry_->list();
    for (auto const& scope : all_scopes)
    {
        // New scopes are added disabled by default
        return_list.push_back( ChildScope{scope.first, false} );
    }
    return return_list;
}

ChildScopeList ScopeBaseImpl::child_scopes_ordered(ChildScopeList const& child_scopes) const
{
    ///!
}

void ScopeBaseImpl::set_child_scopes_ordered(ChildScopeList const& child_scopes_ordered)
{
    ///!
}

} // namespace internal

} // namespace scopes

} // namespace unity

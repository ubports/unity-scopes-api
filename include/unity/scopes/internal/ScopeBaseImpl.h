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

#pragma once

#include <unity/scopes/ChildScope.h>
#include <unity/scopes/RegistryProxyFwd.h>
#include <unity/scopes/Variant.h>

#include <mutex>
#include <string>

namespace unity
{

namespace scopes
{

namespace internal
{

class SettingsDB;

class ScopeBaseImpl final
{
public:
    ScopeBaseImpl();
    ~ScopeBaseImpl() = default;

    void set_scope_directory(std::string const& path);
    std::string scope_directory() const;

    void set_cache_directory(std::string const& path);
    std::string cache_directory() const;

    void set_app_directory(std::string const& path);
    std::string app_directory() const;

    void set_tmp_directory(std::string const& path);
    std::string tmp_directory() const;

    void set_registry(RegistryProxy const& registry);
    RegistryProxy registry() const;

    std::shared_ptr<unity::scopes::internal::SettingsDB> settings_db() const;
    void set_settings_db(std::shared_ptr<unity::scopes::internal::SettingsDB> const& db);
    unity::scopes::VariantMap settings() const;

    unity::scopes::ChildScopeList child_scopes_ordered() const;

private:
    std::string scope_directory_;
    bool scope_dir_initialized_;

    std::string cache_directory_;
    bool cache_dir_initialized_;

    std::string app_directory_;
    bool app_dir_initialized_;

    std::string tmp_directory_;
    bool tmp_dir_initialized_;

    unity::scopes::RegistryProxy registry_;
    bool registry_initialized_;

    std::shared_ptr<unity::scopes::internal::SettingsDB> db_;
    bool settings_db_initialized_;

    mutable std::mutex mutex_;
};

} // namespace internal

} // namespace scopes

} // namespace unity

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

#ifndef UNITY_SCOPES_INTERNAL_SCOPEBASEIMPL_H
#define UNITY_SCOPES_INTERNAL_SCOPEBASEIMPL_H

#include <unity/scopes/Variant.h>

#include <memory>
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
    ScopeBaseImpl() = default;
    ~ScopeBaseImpl() = default;
    void set_scope_directory(std::string const &path);
    std::string scope_directory() const;
    std::shared_ptr<unity::scopes::internal::SettingsDB> settings_db() const;
    void set_settings_db(std::shared_ptr<unity::scopes::internal::SettingsDB> const& db);
    unity::scopes::VariantMap settings() const;

private:
    std::string scope_directory_;
    std::shared_ptr<unity::scopes::internal::SettingsDB> db_;
    mutable std::mutex mutex_;
};

} // namespace internal

} // namespace scopes

} // namespace unity

#endif

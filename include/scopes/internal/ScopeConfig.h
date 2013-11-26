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
 * Authored by: Michi Henning <michi.henning@canonical.com>
 */

#ifndef UNITY_API_SCOPES_INTERNAL_SCOPECONFIG_H
#define UNITY_API_SCOPES_INTERNAL_SCOPECONFIG_H

#include <scopes/internal/ConfigBase.h>

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

class ScopeConfig : public ConfigBase
{
public:
    static constexpr const char* SCOPE_CONFIG_GROUP = "ScopeConfig";

    ScopeConfig(std::string const& configfile);
    ~ScopeConfig() noexcept;

    bool overrideable() const;      // Optional, returns false if not present
    std::string art() const;
    std::string localized_name() const;
    std::string description() const;
    std::string search_hint() const;     // Optional, throws NotFoundException if not present
    std::string hot_key() const;         // Optional, throws NotFoundException if not present

private:
    bool overrideable_;
    std::string art_;
    std::string localized_name_;
    std::string description_;
    std::unique_ptr<std::string> search_hint_;
    std::unique_ptr<std::string> hot_key_;
};

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

#endif

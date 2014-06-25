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

#ifndef UNITY_SCOPES_INTERNAL_SCOPECONFIG_H
#define UNITY_SCOPES_INTERNAL_SCOPECONFIG_H

#include <unity/scopes/internal/ConfigBase.h>
#include <unity/scopes/Variant.h>
#include <unity/scopes/ScopeMetadata.h>

namespace unity
{

namespace scopes
{

namespace internal
{


class ScopeConfig : public ConfigBase
{
public:
    ScopeConfig(std::string const& configfile);
    ~ScopeConfig();

    bool overrideable() const;           // Optional, returns false if not present
    std::string display_name() const;    // Mandatory, localizable
    std::string description() const;     // Mandatory, localizable
    std::string author() const;          // Mandatory
    std::string art() const;             // Optional, throws NotFoundException if not present
    std::string icon() const;            // Optional, throws NotFoundException if not present
    std::string search_hint() const;     // Optional, localizable, throws NotFoundException if not present
    std::string hot_key() const;         // Optional, throws NotFoundException if not present
    bool invisible() const;              // Optional, returns false if not present
    std::string scope_runner() const;    // Optional, throws NotFoundException if not present
    int idle_timeout() const;            // Optional, returns default value if not present

    VariantMap appearance_attributes() const; // Optional, return empty map if no attributes are present
    ScopeMetadata::ResultsTtlType results_ttl_type() const;    // Optional, returns none if not present

private:
    static void parse_appearance_attribute(VariantMap& var, std::string const& key, std::string const& val);

    bool overrideable_;
    std::string display_name_;
    std::string description_;
    std::string author_;
    std::unique_ptr<std::string> art_;
    std::unique_ptr<std::string> icon_;
    std::unique_ptr<std::string> search_hint_;
    std::unique_ptr<std::string> hot_key_;
    bool invisible_;
    std::unique_ptr<std::string> scope_runner_;
    int idle_timeout_;
    VariantMap appearance_attributes_;
    ScopeMetadata::ResultsTtlType results_ttl_type_;
};

} // namespace internal

} // namespace scopes

} // namespace unity

#endif

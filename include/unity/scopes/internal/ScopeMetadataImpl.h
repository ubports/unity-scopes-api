/*
 * Copyright (C) 2013 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the Lesser GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the Lesser GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Michi Henning <michi.henning@canonical.com>
 */

#ifndef UNITY_SCOPES_INTERNAL_SCOPEMETADATAIMPL_H
#define UNITY_SCOPES_INTERNAL_SCOPEMETADATAIMPL_H

#include <unity/scopes/internal/MWScope.h>
#include <unity/scopes/ScopeMetadata.h>

#include <unordered_set>

namespace unity
{

namespace scopes
{

namespace internal
{

class ScopeMetadataImpl
{
public:
    ScopeMetadataImpl(MiddlewareBase* mw);
    ScopeMetadataImpl(VariantMap const& variant_map, MiddlewareBase* mw);
    ScopeMetadataImpl(ScopeMetadataImpl const&);
    ScopeMetadataImpl(ScopeMetadataImpl&&) = default;
    ScopeMetadataImpl& operator=(ScopeMetadataImpl const&);
    ScopeMetadataImpl& operator=(ScopeMetadataImpl&&) = default;

    std::string scope_id() const;
    ScopeProxy proxy() const;
    std::string display_name() const;                            // mandatory, localized
    std::string description() const;                             // mandatory, localized
    std::string author() const;                                  // mandatory
    std::string art() const;                                     // optional
    std::string icon() const;                                    // optional
    std::string search_hint() const;                             // optional, localized
    std::string hot_key() const;                                 // optional
    bool invisible() const;                                      // optional (default: false)
    VariantMap appearance_attributes() const;                    // optional (default: empty map)
    std::string scope_directory() const;                         // optional
    ScopeMetadata::ResultsTtlType results_ttl_type() const;      // optional (default: none)
    VariantArray settings_definitions() const;                   // optional (default: empty array)
    bool location_data_needed() const;                           // optional (default: false)
    std::vector<std::string> child_scope_ids() const;            // optional (default: empty array)

    void set_scope_id(std::string const& scope_id);
    void set_proxy(ScopeProxy const& proxy);
    void set_display_name(std::string const& display_name);
    void set_description(std::string const& description);
    void set_author(std::string const& author);
    void set_art(std::string const& art);
    void set_icon(std::string const& icon);
    void set_search_hint(std::string const& search_hint);
    void set_hot_key(std::string const& hot_key);
    void set_invisible(bool invisible);
    void set_appearance_attributes(VariantMap const& appearance_attributes);
    void set_scope_directory(std::string const& path);
    void set_results_ttl_type(ScopeMetadata::ResultsTtlType results_ttl);
    void set_settings_definitions(VariantArray const& settings_definitions);
    void set_location_data_needed(bool location_data_needed);
    void set_child_scope_ids(std::vector<std::string> const& ids);

    VariantMap serialize() const;
    void deserialize(VariantMap const& var);

    static ScopeMetadata create(std::unique_ptr<ScopeMetadataImpl> impl);

private:

    MiddlewareBase* mw_;
    std::string scope_id_;
    ScopeProxy proxy_;
    std::string display_name_;
    std::string description_;
    std::string author_;
    std::unique_ptr<std::string> art_;                    // Optional, hence a pointer
    std::unique_ptr<std::string> icon_;                   // Optional, hence a pointer
    std::unique_ptr<std::string> search_hint_;            // Optional, hence a pointer
    std::unique_ptr<std::string> hot_key_;                // Optional, hence a pointer
    std::unique_ptr<bool> invisible_;                     // Optional, hence a pointer
    std::unique_ptr<std::string> scope_directory_;        // Optional, hence a pointer
    VariantMap appearance_attributes_;
    ScopeMetadata::ResultsTtlType results_ttl_type_;
    std::unique_ptr<VariantArray> settings_definitions_;  // Optional, hence a pointer
    std::unique_ptr<bool> location_data_needed_;          // Optional, hence a pointer
    std::vector<std::string> child_scope_ids_;
};

} // namespace internal

} // namespace scopes

} // namespace unity


#endif

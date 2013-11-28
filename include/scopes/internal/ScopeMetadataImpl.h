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

#ifndef UNITY_API_SCOPES_INTERNAL_SCOPEMETADATAIMPL_H
#define UNITY_API_SCOPES_INTERNAL_SCOPEMETADATAIMPL_H

#include <scopes/internal/MWScope.h>
#include <scopes/ScopeMetadata.h>

#include <unordered_set>

namespace unity
{

namespace api
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

    std::string scope_name() const;
    std::string art() const;
    ScopeProxy proxy() const;
    std::string localized_name() const; // localized
    std::string description() const;    // localized
    std::string search_hint() const;    // localized, optional
    std::string hot_key() const;        // localized, optional

    void set_scope_name(std::string const& scope_name);
    void set_art(std::string const& art);
    void set_proxy(ScopeProxy const& proxy);
    void set_localized_name(std::string const& localized_name);
    void set_description(std::string const& description);
    void set_search_hint(std::string const& search_hint);
    void set_hot_key(std::string const& hot_key);

    VariantMap serialize() const;
    void deserialize(VariantMap const& var);

    static ScopeMetadata create(std::unique_ptr<ScopeMetadataImpl> impl);

private:

    MiddlewareBase* mw_;
    std::string scope_name_;
    std::string art_;
    ScopeProxy proxy_;
    std::string localized_name_;
    std::string description_;
    std::unique_ptr<std::string> search_hint_;  // Optional, hence a pointer
    std::unique_ptr<std::string> hot_key_;      // Optional, hence a pointer
};

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity


#endif

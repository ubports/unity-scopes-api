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
 * Authored by: Thomas Voß <thomas.voss@canonical.com>
 */

#include <unity/scopes/testing/ScopeMetadataBuilder.h>

#include <unity/scopes/internal/MiddlewareBase.h>
#include <unity/scopes/internal/ScopeMetadataImpl.h>

namespace internal = unity::scopes::internal;
namespace testing = unity::scopes::testing;

/// @cond

struct testing::ScopeMetadataBuilder::Private
{
    constexpr static unity::scopes::internal::MiddlewareBase* invalid_middleware = nullptr;

    std::string scope_id;
    ScopeProxy proxy;
    std::string display_name;
    std::string description;
    std::string author;

    Optional<std::string> art;
    Optional<std::string> icon;
    Optional<std::string> search_hint;
    Optional<std::string> hot_key;
    Optional<bool> invisible;
    Optional<VariantMap> appearance_attributes;
    Optional<std::string> scope_directory;
    Optional<ScopeMetadata::ResultsTtlType> results_ttl_type;
    Optional<VariantArray> settings_definitions;
    Optional<bool> location_data_needed;
    Optional<std::vector<std::string>> child_scope_ids;
    Optional<int> version;
};

testing::ScopeMetadataBuilder::ScopeMetadataBuilder() : p(new Private())
{
}

testing::ScopeMetadataBuilder::~ScopeMetadataBuilder()
{
}

testing::ScopeMetadataBuilder& testing::ScopeMetadataBuilder::scope_id(std::string const& value)
{
    p->scope_id = value;
    return *this;
}

testing::ScopeMetadataBuilder& testing::ScopeMetadataBuilder::proxy(ScopeProxy const& value)
{
    p->proxy = value;
    return *this;
}

testing::ScopeMetadataBuilder& testing::ScopeMetadataBuilder::display_name(std::string const& value)
{
    p->display_name = value;
    return *this;
}

testing::ScopeMetadataBuilder& testing::ScopeMetadataBuilder::description(std::string const& value)
{
    p->description = value;
    return *this;
}

testing::ScopeMetadataBuilder& testing::ScopeMetadataBuilder::author(std::string const& value)
{
    p->author = value;
    return *this;
}

testing::ScopeMetadataBuilder& testing::ScopeMetadataBuilder::art(Optional<std::string> const& value)
{
    p->art = value;
    return *this;
}

testing::ScopeMetadataBuilder& testing::ScopeMetadataBuilder::icon(Optional<std::string> const& value)
{
    p->icon = value;
    return *this;
}

testing::ScopeMetadataBuilder& testing::ScopeMetadataBuilder::search_hint(Optional<std::string> const& value)
{
    p->search_hint = value;
    return *this;
}

testing::ScopeMetadataBuilder& testing::ScopeMetadataBuilder::hot_key(Optional<std::string> const& value)
{
    p->hot_key = value;
    return *this;
}

testing::ScopeMetadataBuilder& testing::ScopeMetadataBuilder::invisible(Optional<bool> const& value)
{
    p->invisible = value;
    return *this;
}

testing::ScopeMetadataBuilder& testing::ScopeMetadataBuilder::appearance_attributes(Optional<VariantMap> const& value)
{
    p->appearance_attributes = value;
    return *this;
}

testing::ScopeMetadataBuilder& testing::ScopeMetadataBuilder::scope_directory(Optional<std::string> const& value)
{
    p->scope_directory = value;
    return *this;
}

testing::ScopeMetadataBuilder& testing::ScopeMetadataBuilder::results_ttl_type(Optional<ScopeMetadata::ResultsTtlType> const& value)
{
    p->results_ttl_type = value;
    return *this;
}

testing::ScopeMetadataBuilder& testing::ScopeMetadataBuilder::settings_definitions(Optional<VariantArray> const& value)
{
    p->settings_definitions = value;
    return *this;
}

testing::ScopeMetadataBuilder& testing::ScopeMetadataBuilder::location_data_needed(Optional<bool> const& value)
{
    p->location_data_needed = value;
    return *this;
}

testing::ScopeMetadataBuilder& testing::ScopeMetadataBuilder::child_scope_ids(Optional<std::vector<std::string>> const& value)
{
    p->child_scope_ids = value;
    return *this;
}

testing::ScopeMetadataBuilder& testing::ScopeMetadataBuilder::version(Optional<int> const& value)
{
    p->version = value;
    return *this;
}

unity::scopes::ScopeMetadata testing::ScopeMetadataBuilder::operator()() const
{
    auto impl = new unity::scopes::internal::ScopeMetadataImpl(Private::invalid_middleware);
    impl->set_scope_id(p->scope_id);
    impl->set_proxy(p->proxy);
    impl->set_display_name(p->display_name);
    impl->set_description(p->description);
    impl->set_author(p->author);

    if (p->art)
        impl->set_art(*p->art);
    if (p->icon)
        impl->set_icon(*p->icon);
    if (p->search_hint)
        impl->set_search_hint(*p->search_hint);
    if (p->hot_key)
        impl->set_hot_key(*p->hot_key);
    if (p->invisible)
        impl->set_invisible(*p->invisible);
    if (p->appearance_attributes)
        impl->set_appearance_attributes(*p->appearance_attributes);
    if (p->scope_directory)
        impl->set_scope_directory(*p->scope_directory);
    if (p->results_ttl_type)
        impl->set_results_ttl_type(*p->results_ttl_type);
    if (p->settings_definitions)
        impl->set_settings_definitions(*p->settings_definitions);
    if (p->location_data_needed)
        impl->set_location_data_needed(*p->location_data_needed);
    if (p->child_scope_ids)
        impl->set_child_scope_ids(*p->child_scope_ids);
    if (p->version)
        impl->set_version(*p->version);

    return unity::scopes::internal::ScopeMetadataImpl::create(
                std::move(
                    std::unique_ptr<internal::ScopeMetadataImpl>{impl}));
}

/// @endcond

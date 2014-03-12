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

    Optional<std::string> art;
    Optional<std::string> icon;
    Optional<std::string> search_hint;
    Optional<std::string> hot_key;
    Optional<bool> invisible;
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

testing::ScopeMetadataBuilder& testing::ScopeMetadataBuilder::invisible(Optional<bool> value)
{
    p->invisible = value;
    return *this;
}

unity::scopes::ScopeMetadata testing::ScopeMetadataBuilder::operator()() const
{
    auto impl = new unity::scopes::internal::ScopeMetadataImpl(Private::invalid_middleware);
    impl->set_scope_id(p->scope_id);
    impl->set_proxy(p->proxy);
    impl->set_display_name(p->display_name);
    impl->set_description(p->description);

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

    return unity::scopes::internal::ScopeMetadataImpl::create(
                std::move(
                    std::unique_ptr<internal::ScopeMetadataImpl>{impl}));
}

/// @endcond

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
 * Lesser GNU General Public License for more details.
 *
 * You should have received a copy of the Lesser GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Pawel Stolowski <pawel.stolowski@canonical.com>
 */

#include <scopes/internal/ResultItemImpl.h>
#include <scopes/ScopeExceptions.h>
#include <unity/UnityExceptions.h>
#include <scopes/Category.h>

#include <cassert>

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

ResultItemImpl::ResultItemImpl(Category::SPtr category)
    : category_(category)
{
    if (category_ == nullptr)
    {
        throw InvalidArgumentException("ResultItemImpl(): null category");
    }
}

ResultItemImpl::ResultItemImpl(Category::SPtr category, const VariantMap& variant_map)
    : ResultItemImpl(category)
{
    from_variant_map(variant_map);
}

void ResultItemImpl::set_uri(std::string const& uri)
{
    uri_ = uri;
}

void ResultItemImpl::set_title(std::string const& title)
{
    title_ = title;
}

void ResultItemImpl::set_icon(std::string const& icon)
{
    icon_ = icon;
}

void ResultItemImpl::set_dnd_uri(std::string const& dnd_uri)
{
    dnd_uri_ = dnd_uri;
}

void ResultItemImpl::add_metadata(std::string const& key, Variant const& value)
{
    //TODO
}

std::string ResultItemImpl::uri() const
{
    return uri_;
}

std::string ResultItemImpl::title() const
{
    return title_;
}

std::string ResultItemImpl::icon() const
{
    return icon_;
}

std::string ResultItemImpl::dnd_uri() const
{
    return dnd_uri_;
}

Category::SPtr ResultItemImpl::category() const
{
    return category_;
}

VariantMap ResultItemImpl::to_variant_map() const
{
    VariantMap var;
    var["uri"] = uri_;
    var["title"] = title_;
    var["icon"] = icon_;
    var["dnd_uri"] = dnd_uri_;
    var["cat_id"] = category_->id();
    return var;
}

void ResultItemImpl::from_variant_map(VariantMap const& var)
{
    auto it = var.find("uri");
    if (it == var.end())
        throw MiddlewareException("Missing 'uri'");
    uri_ = it->second.get_string();

    it = var.find("title");
    if (it == var.end())
        throw MiddlewareException("Missing 'title'");
    title_ = it->second.get_string();

    it = var.find("icon");
    if (it == var.end())
        throw MiddlewareException("Missing 'icon'");
    icon_ = it->second.get_string();

    it = var.find("dnd_uri");
    if (it == var.end())
        throw MiddlewareException("Missing 'dnd_uri'");
    dnd_uri_ = it->second.get_string();
}

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

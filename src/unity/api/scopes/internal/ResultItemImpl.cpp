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

#include <unity/api/scopes/internal/ResultItemImpl.h>
#include <unity/api/scopes/ScopeExceptions.h>

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

void ResultItemImpl::set_uri(std::string const& uri)
{
    this->uri = uri;
}

void ResultItemImpl::set_title(std::string const& title)
{
    this->title = title;
}

void ResultItemImpl::set_icon(std::string const& icon)
{
    this->icon = icon;
}

void ResultItemImpl::set_dnd_uri(std::string const& dnd_uri)
{
    this->dnd_uri = dnd_uri;
}

void ResultItemImpl::set_renderer_hint(std::string const& name, Variant const& value)
{
    render_hints[name] = value;
}

std::string ResultItemImpl::get_uri() const
{
    return uri;
}

std::string ResultItemImpl::get_title() const
{
    return title;
}

std::string ResultItemImpl::get_icon() const
{
    return icon;
}

std::string ResultItemImpl::get_dnd_uri() const
{
    return dnd_uri;
}

const VariantMap ResultItemImpl::to_variant() const
{
    VariantMap var;
    var["uri"] = uri;
    var["title"] = title;
    var["icon"] = icon;
    var["dnd_uri"] = dnd_uri;
    var["render_hints"] = render_hints;
    return var;
}

void ResultItemImpl::from_variant(VariantMap const& var)
{
    auto it = var.find("uri");
    if (it == var.end())
        throw MiddlewareException("Missing 'uri'");
    uri = it->second.get_string();

    it = var.find("title");
    if (it == var.end())
        throw MiddlewareException("Missing 'title'");
    title = it->second.get_string();

    it = var.find("icon");
    if (it == var.end())
        throw MiddlewareException("Missing 'icon'");
    icon = it->second.get_string();

    it = var.find("dnd_uri");
    if (it == var.end())
        throw MiddlewareException("Missing 'dnd_uri'");
    dnd_uri = it->second.get_string();

    it = var.find("render_hints");
    if (it == var.end())
        throw MiddlewareException("Missing 'render_hints'");
    render_hints = it->second.get_dict();
}

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

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

#include <unity/api/scopes/ResultItem.h>
#include <unity/api/scopes/internal/ResultItemImpl.h>

namespace unity
{

namespace api
{

namespace scopes
{

//! @cond

ResultItem::ResultItem(std::shared_ptr<Category> category)
    : p(new internal::ResultItemImpl(category))
{
}

void ResultItem::set_uri(std::string const& uri)
{
    p->set_uri(uri);
}

void ResultItem::set_title(std::string const& title)
{
    p->set_title(title);
}

void ResultItem::set_icon(std::string const& icon)
{
    p->set_icon(icon);
}

void ResultItem::set_dnd_uri(std::string const& dnd_uri)
{
    p->set_dnd_uri(dnd_uri);
}

void ResultItem::set_renderer_hint(std::string const& name, Variant const& value)
{
    p->set_renderer_hint(name, value);
}

std::string ResultItem::get_uri() const
{
    return p->get_uri();
}

std::string ResultItem::get_title() const
{
    return p->get_title();
}

std::string ResultItem::get_icon() const
{
    return p->get_icon();
}

std::string ResultItem::get_dnd_uri() const
{
    return p->get_dnd_uri();
}

//! @endcond

} // namespace scopes

} // namespace api

} // namespace unity

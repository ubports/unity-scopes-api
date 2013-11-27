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
 * Authored by: Pawel Stolowski <pawel.stolowski@canonical.com>
 */

#include <scopes/ResultItem.h>
#include <scopes/internal/ResultItemImpl.h>

namespace unity
{

namespace api
{

namespace scopes
{

//! @cond

ResultItem::ResultItem(internal::ResultItemImpl* impl)
    : p(impl)
{
}

ResultItem::ResultItem(const VariantMap &variant_map)
    : p(new internal::ResultItemImpl(variant_map))
{
}

ResultItem::ResultItem(ResultItem const& other)
    : p(new internal::ResultItemImpl(*(other.p)))
{
}

ResultItem::~ResultItem()
{
}

ResultItem& ResultItem::operator=(ResultItem const& other)
{
    if (this != &other)
    {
        p = std::make_shared<internal::ResultItemImpl>(*(other.p));
    }
    return *this;
}

ResultItem::ResultItem(ResultItem&&) = default;

ResultItem& ResultItem::operator=(ResultItem&&) = default;

void ResultItem::store(ResultItem const& other)
{
    p->store(other);
}

bool ResultItem::has_stored_result() const
{
    return p->has_stored_result();
}

ResultItem ResultItem::retrieve() const
{
    return p->retrieve();
}

void ResultItem::set_uri(std::string const& uri)
{
    p->set_uri(uri);
}

void ResultItem::set_title(std::string const& title)
{
    p->set_title(title);
}

void ResultItem::set_art(std::string const& image)
{
    p->set_art(image);
}

void ResultItem::set_dnd_uri(std::string const& dnd_uri)
{
    p->set_dnd_uri(dnd_uri);
}

void ResultItem::add_metadata(std::string const& key, Variant const& value)
{
    p->add_metadata(key, value);
}

std::string ResultItem::uri() const
{
    return p->uri();
}

std::string ResultItem::title() const
{
    return p->title();
}

std::string ResultItem::art() const
{
    return p->art();
}

std::string ResultItem::dnd_uri() const
{
    return p->dnd_uri();
}

VariantMap ResultItem::serialize() const
{
    return p->serialize();
}

//! @endcond

} // namespace scopes

} // namespace api

} // namespace unity

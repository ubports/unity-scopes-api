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

#include <scopes/internal/CategoryImpl.h>
#include <unity/UnityExceptions.h>

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

//! @cond

CategoryImpl::CategoryImpl(VariantMap const& variant_map)
{
    deserialize(variant_map);
}

CategoryImpl::CategoryImpl(std::string const& id, std::string const& title, std::string const &icon, CategoryRenderer const& renderer_template)
    : id_(id),
      title_(title),
      icon_(icon),
      renderer_template_(renderer_template)
{
    if (id.empty())
    {
        throw InvalidArgumentException("Category id must not be empty");
    }
    // it's ok if title and icon are empty.
}

std::string const& CategoryImpl::id() const
{
    return id_;
}

std::string const& CategoryImpl::title() const
{
    return title_;
}

std::string const& CategoryImpl::icon() const
{
    return icon_;
}

CategoryRenderer const& CategoryImpl::renderer_template() const
{
    return renderer_template_;
}

VariantMap CategoryImpl::serialize() const
{
    VariantMap var;
    var["id"] = id_;
    var["title"] = title_;
    var["icon"] = icon_;
    var["renderer_template"] = renderer_template_.data();
    return var;
}

void CategoryImpl::deserialize(VariantMap const& variant_map)
{
    auto it = variant_map.find("id");
    if (it == variant_map.end())
    {
        throw InvalidArgumentException("Missing 'id'");
    }
    id_ = it->second.get_string();

    it = variant_map.find("title");
    if (it != variant_map.end())
    {
        title_ = it->second.get_string();
    }

    it = variant_map.find("icon");
    if (it != variant_map.end())
    {
        icon_ = it->second.get_string();
    }

    it = variant_map.find("renderer_template");
    if (it != variant_map.end())
    {
        renderer_template_ = CategoryRenderer(it->second.get_string()); // can throw if json is invalid
    }
}

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

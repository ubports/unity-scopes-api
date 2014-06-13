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

#include <unity/scopes/internal/CategoryImpl.h>
#include <unity/scopes/internal/Utils.h>
#include <unity/UnityExceptions.h>

namespace unity
{

namespace scopes
{

namespace internal
{

CategoryImpl::CategoryImpl(VariantMap const& variant_map)
{
    deserialize(variant_map);
}

CategoryImpl::CategoryImpl(std::string const& id, std::string const& title, std::string const &icon, CategoryRenderer const& renderer_template, Category::TapBehavior
            tap_behavior, Category::TapBehavior long_press_behavior)
    : id_(id),
      title_(title),
      icon_(icon),
      tap_behavior_(tap_behavior),
      long_press_behavior_(long_press_behavior),
      renderer_template_(renderer_template)
{
    if (id.empty())
    {
        throw InvalidArgumentException("Category id must not be empty");
    }
    // it's ok if title and icon are empty.
}

CategoryImpl::CategoryImpl(std::string const& id, std::string const& title, std::string const &icon, CategoryRenderer const& renderer_template)
    : CategoryImpl(id, title, icon, renderer_template, Category::TapBehavior::TapPreview, Category::TapBehavior::TapPreview)
{
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

Category::TapBehavior CategoryImpl::tap_behavior() const
{
    return tap_behavior_;
}

Category::TapBehavior CategoryImpl::long_press_behavior() const
{
    return long_press_behavior_;
}

VariantMap CategoryImpl::serialize() const
{
    VariantMap var;
    var["id"] = id_;
    var["title"] = title_;
    var["icon"] = icon_;
    var["renderer_template"] = renderer_template_.data();
    var["tap_behavior"] = static_cast<int>(tap_behavior_);
    var["long_press_behavior"] = static_cast<int>(long_press_behavior_);
    return var;
}

void CategoryImpl::deserialize(VariantMap const& variant_map)
{
    auto it = find_or_throw("CategoryImpl::deserialize()", variant_map, "id");
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

    it = find_or_throw("CategoryImpl::deserialize()", variant_map, "tap_behavior");
    tap_behavior_ = static_cast<Category::TapBehavior>(it->second.get_int());
    it = find_or_throw("CategoryImpl::deserialize()", variant_map, "long_press_behavior");
    long_press_behavior_ = static_cast<Category::TapBehavior>(it->second.get_int());
}

} // namespace internal

} // namespace scopes

} // namespace unity

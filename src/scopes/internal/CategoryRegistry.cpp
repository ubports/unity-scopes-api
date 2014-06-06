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

#include <unity/scopes/internal/CategoryRegistry.h>
#include <unity/scopes/internal/CategoryImpl.h>
#include <unity/UnityExceptions.h>
#include <sstream>

namespace unity
{

namespace scopes
{

namespace internal
{

void CategoryRegistry::register_category(Category::SCPtr category)
{
    std::lock_guard<decltype(mutex_)> lock(mutex_);
    if (categories_.find(category->id()) != categories_.end())
    {
        std::ostringstream s;
        s << "Category " << category->id() << " already defined";
        throw InvalidArgumentException(s.str());
    }
    categories_[category->id()] = category;
}

Category::SCPtr CategoryRegistry::register_category(VariantMap const& variant_map)
{
    auto cat = std::shared_ptr<Category>(new Category(variant_map));
    register_category(cat);
    return cat;
}

Category::SCPtr CategoryRegistry::register_category(std::string const& id, std::string const& title, std::string const& icon, CategoryRenderer const&
        renderer_template, Category::TapBehavior single_tap_behavior, Category::TapBehavior long_tap_behavior)
{
    auto cat = std::shared_ptr<Category>(new Category(new internal::CategoryImpl(id, title, icon, renderer_template, single_tap_behavior, long_tap_behavior)));
    register_category(cat);
    return cat;
}

Category::SCPtr CategoryRegistry::lookup_category(std::string const& id) const
{
    std::lock_guard<decltype(mutex_)> lock(mutex_);
    auto it = categories_.find(id);
    if (it != categories_.end())
    {
        return it->second;
    }
    return nullptr;
}

} // namespace internal

} // namespace scopes

} // namespace unity

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
#include <unity/UnityExceptions.h>

#include <algorithm>
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
    auto id = category->id();
    auto it = find_if(categories_.begin(), categories_.end(), [id](const CatPair& p) { return p.first == id; });
    if (it != categories_.end())
    {
        throw InvalidArgumentException("register_category(): duplicate category: " + id);
    }
    categories_.push_back(make_pair(category->id(), category));
}

Category::SCPtr CategoryRegistry::register_category(VariantMap const& variant_map)
{
    auto cat = std::shared_ptr<Category>(new Category(variant_map));
    register_category(cat);
    return cat;
}

Category::SCPtr CategoryRegistry::register_category(std::string const& id, std::string const& title, std::string const& icon, CannedQuery::SCPtr const& query, CategoryRenderer const& renderer_template)
{
    auto cat = std::shared_ptr<Category>(new Category(id, title, icon, query, renderer_template));
    register_category(cat);
    return cat;
}

Category::SCPtr CategoryRegistry::lookup_category(std::string const& id) const
{
    std::lock_guard<decltype(mutex_)> lock(mutex_);
    auto it = find_if(categories_.begin(), categories_.end(), [id](const CatPair& p) { return p.first == id; });
    if (it != categories_.end())
    {
        return it->second;
    }
    return nullptr;
}

VariantArray CategoryRegistry::serialize() const
{
    std::lock_guard<decltype(mutex_)> lock(mutex_);
    VariantArray va;
    for (auto&& p : categories_)
    {
        va.push_back(Variant(p.second->serialize()));
    }
    return va;
}

} // namespace internal

} // namespace scopes

} // namespace unity

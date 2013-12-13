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

#include <scopes/internal/CategorisedResultImpl.h>
#include <scopes/internal/CategoryRegistry.h>
#include <unity/UnityExceptions.h>
#include <scopes/Category.h>
#include <sstream>

namespace unity
{

namespace scopes
{

namespace internal
{

CategorisedResultImpl::CategorisedResultImpl(CategorisedResultImpl const& other)
    : ResultImpl(other)
{
    set_category(other.category_);
}

CategorisedResultImpl::CategorisedResultImpl(Category::SCPtr category)
    : ResultImpl()
{
    set_category(category);
}

CategorisedResultImpl::CategorisedResultImpl(Category::SCPtr category, const VariantMap& variant_map)
    : ResultImpl(variant_map)
{
    set_category(category);
}

CategorisedResultImpl::CategorisedResultImpl(internal::CategoryRegistry const& reg, const VariantMap &variant_map)
    : ResultImpl(variant_map)
{
    auto it = variant_map.find("internal");
    if (it == variant_map.end())
    {
        throw InvalidArgumentException("Invalid variant, missing 'internal'");
    }
    auto cat_id = it->second.get_dict()["cat_id"].get_string();
    category_ = reg.lookup_category(cat_id);
    if (category_ == nullptr)
    {
        std::ostringstream s;
        s << "Category '" << cat_id << "' not found in the registry";
        throw InvalidArgumentException(s.str());
    }
}

Category::SCPtr CategorisedResultImpl::category() const
{
    return category_;
}

void CategorisedResultImpl::set_category(Category::SCPtr category)
{
    if (category == nullptr)
    {
        throw InvalidArgumentException("Category must not be null");
    }
    category_ = category;
}

void CategorisedResultImpl::serialize_internal(VariantMap& var) const
{
    ResultImpl::serialize_internal(var);
    var["cat_id"] = category_->id();
}

} // namespace internal

} // namespace scopes

} // namespace unity

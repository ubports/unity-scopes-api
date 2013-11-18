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

#ifndef UNITY_API_SCOPES_CATEGORYREGISTRY_H
#define UNITY_API_SCOPES_CATEGORYREGISTRY_H

#include <scopes/Category.h>
#include <scopes/Variant.h>
#include <string>
#include <memory>
#include <mutex>

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

/**
\brief Provides factory method for creating Category instances and keeps track of them.
This class is for internal use, most of its functionality is exposed via register_category and find_category method
of Reply class.
*/
class CategoryRegistry
{
public:
    CategoryRegistry() = default;

    /**
    \brief Deserializes category from a variant_map and registers it. Throws if category with same id exists.
    \return category instance
    */
    Category::SCPtr register_category(VariantMap const& variant_map);

    /**
    \brief Creates category from supplied parameters. Throws if category with same id exists.
    \return category instance
    */
    Category::SCPtr register_category(std::string const& id, std::string const& title, std::string const& icon, std::string const& renderer_template);

    /**
    \brief Finds category instance with give id.
    \return category instance or nullptr if not such category has been registered
    */
    Category::SCPtr find_category(std::string const& id) const;

    /**
    \brief Register an existing category instance with this registry.
    Throws if category with sane id exists.
    */
    void register_category(Category::SCPtr category);

private:
    mutable std::recursive_mutex mutex_;
    std::map<std::string, Category::SCPtr> categories_;
};

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

#endif

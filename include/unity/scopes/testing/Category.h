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
 * Authored by: Thomas Voß <thomas.voss@canonical.com>
 */

#pragma once

#include <unity/scopes/Category.h>

namespace unity
{

namespace scopes
{
class CategoryRenderer;

namespace testing
{

/**
\brief Implementation of Category class for testing.

This class inherits from the unity::scopes::Category and provides a public constructor that allows for creating categories with no need
to register them via a real instance of unity::scopes::SearchReply.
*/
class Category : public unity::scopes::Category
{
public:
    /**
     \brief Creates category instance.

     \param id The identifier of this category
     \param title The title of this category
     \param icon The icon of this category
     \param renderer_template The renderer template
     */
    inline Category(std::string const& id, std::string const& title, std::string const &icon, CategoryRenderer const& renderer_template) :
        unity::scopes::Category(id, title, icon, nullptr, renderer_template)
    {
    }

    /**
     \brief Deserializes category from a variant data.

     \param variant_map Serialized category
     */
    inline Category(VariantMap const& variant_map) :
        unity::scopes::Category(variant_map)
    {
    }
};

} // namespace testing

} // namespace scopes

} // namespace unity

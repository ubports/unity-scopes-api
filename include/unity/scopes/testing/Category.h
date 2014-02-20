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

#ifndef UNITY_SCOPES_TESTING_CATEGORY_H
#define UNITY_SCOPES_TESTING_CATEGORY_H

#include <unity/scopes/Category.h>

namespace unity
{

namespace scopes
{
class CategoryRenderer;

namespace testing
{

/**
   \brief Category represents a set of related results returned by scope
   and displayed within a single pane in the Unity dash.
   To create a Category, use ReplyProxy::register_category.
   \see ResultItem
*/
class Category : public unity::scopes::Category
{
public:
    inline Category(std::string const& id, std::string const& title, std::string const &icon, CategoryRenderer const& renderer_template) :
        unity::scopes::Category(id, title, icon, renderer_template)
    {
    }

    inline Category(VariantMap const& variant_map) :
        unity::scopes::Category(variant_map)
    {
    }
};

} // namespace testing

} // namespace scopes

} // namespace unity

#endif

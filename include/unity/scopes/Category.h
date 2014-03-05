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

#ifndef UNITY_SCOPES_CATEGORY_H
#define UNITY_SCOPES_CATEGORY_H

#include <unity/util/NonCopyable.h>
#include <unity/util/DefinesPtrs.h>
#include <unity/scopes/Variant.h>
#include <string>
#include <memory>

namespace unity
{

namespace scopes
{
    class CategoryRenderer;

namespace internal
{
    class CategoryImpl;
    class CategoryRegistry;
}

/**
   \brief A set of related results returned by a scope
   and displayed within a single pane in the Unity dash.

   To create a Category, use ReplyProxy::register_category.
   \see ResultItem
*/
class Category
{
public:
    /// @cond
    NONCOPYABLE(Category);
    UNITY_DEFINES_PTRS(Category);

    virtual ~Category();
    /// @endcond

    /**
     \brief Get identifier of this Category
     \return The category identifier.
    */
    std::string id() const;

    /**
     \brief Get title of this Category
     \return The category title.
    */
    std::string title() const;

    /**
     \brief Get icon of this Category
     \return Teh category icon.
    */
    std::string icon() const;

    /**
     \brief Get renderer template of this Category
     \return The category renderer template.
     */
    CategoryRenderer const& renderer_template() const;

    // @cond
    VariantMap serialize() const;
    // @endcond

protected:
    /// @cond
    Category(std::string const& id, std::string const& title, std::string const &icon, CategoryRenderer const& renderer_template);
    Category(VariantMap const& variant_map);
    /// @endcond

    friend class internal::CategoryRegistry;

private:
    std::unique_ptr<internal::CategoryImpl> p;
};

} // namespace scopes

} // namespace unity

#endif

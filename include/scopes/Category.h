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

#ifndef UNITY_API_SCOPES_CATEGORY_H
#define UNITY_API_SCOPES_CATEGORY_H

#include <unity/util/NonCopyable.h>
#include <unity/util/DefinesPtrs.h>
#include <scopes/Variant.h>
#include <string>
#include <memory>

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{
    class CategoryImpl;
    class CategoryRegistry;
}

/**
   \brief Category represents a set of related results returned by scope
   and displayed within a single pane in the Unity dash.
   To create a Category, use ReplyProxy::register_category.
   \see ResultItem
*/
class UNITY_API Category : private util::NonCopyable
{
public:
/// @cond
    UNITY_DEFINES_PTRS(Category);
/// @endcond

    std::string id() const;
    std::string title() const;
    std::string icon() const;
    std::string renderer_template() const;
    std::shared_ptr<VariantMap> serialize() const;

private:
    Category(std::string const& id, std::string const& title, std::string const &icon, std::string const& renderer_template);
    Category(VariantMap const& variant_map);

    std::shared_ptr<internal::CategoryImpl> p;

    friend class internal::CategoryRegistry;
};

} // namespace scopes

} // namespace api

} // namespace unity

#endif

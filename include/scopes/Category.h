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
}

/**
   \brief Category represents a set of related results returned by a scope
   and displayed within a single pane in the Unity dash.
   \see ResultItem
*/
class UNITY_API Category : private util::NonCopyable
{
public:
/// @cond
    UNITY_DEFINES_PTRS(Category);
/// @endcond

    Category(std::string const& id); //TODO: pass renderer
    std::string const& get_id() const;

private:
    std::shared_ptr<internal::CategoryImpl> p;
};

} // namespace scopes

} // namespace api

} // namespace unity

#endif

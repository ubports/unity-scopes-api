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

#ifndef UNITY_API_SCOPES_RESULT_H
#define UNITY_API_SCOPES_RESULT_H

#include <scopes/ResultItem.h>
#include <scopes/Category.h>

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{
    class ResultImpl;
    class ReplyObject;
}

/**
\brief Result is a ResultItem plus a category it belongs to.
*/

class UNITY_API Result : public ResultItem
{
public:
    /**
     \brief Creates a Result instance assigned to given category, with all base attributes initially empty.
     */
    explicit Result(Category::SCPtr category);

    /**
       \brief Creates a Result instance assigned to given category and using values of all base attributes from a variant_map dictionary.
    */
    Result(Category::SCPtr category, const VariantMap &variant_map);

    Result(Result const& other);
    Result& operator=(Result const& other);
    Result(Result&&);
    Result& operator=(Result&&);

    /**
     \brief Return category of this result.
     Get category instance this result belongs to.
     \return category instance
     */
    Category::SCPtr category() const;

private:
    Result(const VariantMap &variant_map, internal::CategoryRegistry const& reg);
    internal::ResultImpl* fwd() const;

    friend class internal::ReplyObject;
};

} // namespace scopes

} // namespace api

} // namespace unity

#endif

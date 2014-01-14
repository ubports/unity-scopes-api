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

#ifndef UNITY_API_SCOPES_CATEGORISEDRESULT_H
#define UNITY_API_SCOPES_CATEGORISEDRESULT_H

#include <scopes/Result.h>
#include <scopes/Category.h>

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{
    class CategorisedResultImpl;
    class ResultReplyObject;
}

/**
\brief CategorisedResult is a Result plus a category it belongs to.
*/

class UNITY_API CategorisedResult: public Result
{
public:
    /**
     \brief Creates a CategorisedResult instance assigned to given category, with all base attributes initially empty.
     */
    explicit CategorisedResult(Category::SCPtr category);

    /**
       \brief Creates a CategorisedResult instance assigned to given category and using values of all base attributes from a variant_map dictionary.
    */
    CategorisedResult(Category::SCPtr category, const VariantMap &variant_map);

    CategorisedResult(CategorisedResult const& other);
    CategorisedResult& operator=(CategorisedResult const& other);
    CategorisedResult(CategorisedResult&&);
    CategorisedResult& operator=(CategorisedResult&&);

    /**
     \brief Return category of this result.
     Get category instance this result belongs to.
     \return category instance
     */
    Category::SCPtr category() const;

private:
    CategorisedResult(std::shared_ptr<internal::ResultImpl> impl);
    internal::CategorisedResultImpl* fwd() const;

    friend class internal::CategorisedResultImpl;
    friend class internal::ResultReplyObject;
};

} // namespace scopes

} // namespace api

} // namespace unity

#endif

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

#ifndef UNITY_SCOPES_CATEGORISEDRESULT_H
#define UNITY_SCOPES_CATEGORISEDRESULT_H

#include <unity/scopes/Result.h>
#include <unity/scopes/Category.h>

namespace unity
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
class CategorisedResult: public Result
{
public:
    /// @cond
    UNITY_DEFINES_PTRS(CategorisedResult);
    /// @endcond

    /**
     \brief Creates a CategorisedResult instance assigned to given category, with all base attributes initially empty.
     */
    explicit CategorisedResult(Category::SCPtr category);

    /// @cond
    CategorisedResult(CategorisedResult const& other);
    CategorisedResult& operator=(CategorisedResult const& other);
    CategorisedResult(CategorisedResult&&);
    CategorisedResult& operator=(CategorisedResult&&);
    /// @endcond

    /**
     \brief Set category of this result.
     */
    void set_category(Category::SCPtr category);

    /**
     \brief Return category of this result.
     Get category instance this result belongs to.
     \return category instance
     */
    Category::SCPtr category() const;

private:
    CategorisedResult(internal::ResultImpl* impl);
    internal::CategorisedResultImpl* fwd() const;

    friend class internal::CategorisedResultImpl;
    friend class internal::ResultReplyObject;
};

} // namespace scopes

} // namespace unity

#endif

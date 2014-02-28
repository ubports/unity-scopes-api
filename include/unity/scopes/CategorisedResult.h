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
\brief A result, including the category it belongs to.
*/

class CategorisedResult: public Result
{
public:
    /// @cond
    UNITY_DEFINES_PTRS(CategorisedResult);
    /// @endcond

    /**
    \brief Creates a CategorisedResult with given category,
    with all base attributes initially empty.
    \param category The category for the result.
    */
    explicit CategorisedResult(Category::SCPtr category);

    /**@name Copy and assignment
    Copy and assignment operators (move and non-move versions) have the usual value semantics.
    */
    //{@
    CategorisedResult(CategorisedResult const& other);
    CategorisedResult& operator=(CategorisedResult const& other);
    CategorisedResult(CategorisedResult&&);
    CategorisedResult& operator=(CategorisedResult&&);
    //@}

    /**
    \brief Updates the category of this result.
    \param category The category for the result.
    */
    void set_category(Category::SCPtr category);

    /**
    \brief Return category of this result.
    Get the category instance this result belongs to.
    \return The category instance.
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

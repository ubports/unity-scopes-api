/*
 * Copyright (C) 2015 Canonical Ltd
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
 * Authored by: Xavi Garcia <xavi.garcia.mena@canonical.com>
 */

#pragma once

#include <unity/scopes/qt/QResult.h>
#include <unity/scopes/qt/QCategory.h>

namespace unity
{

namespace scopes
{

namespace qt
{

namespace internal
{
class QSearchReplyImpl;
class QCategorisedResultImpl;
}

/**
\brief A result, including the category it belongs to.
*/

class QCategorisedResult : public QResult
{
public:
    /// @cond
    UNITY_DEFINES_PTRS(QCategorisedResult);
    /// @endcond

    /**
    \brief Creates a CategorisedResult with given category,
    with all base attributes initially empty.
    \param category The category for the result.
    */
    explicit QCategorisedResult(QCategory::SCPtr category);

    /**@name Copy and assignment
    Copy and assignment operators (move and non-move versions) have the usual value semantics.
    */
    //{@
    QCategorisedResult(QCategorisedResult const& other);
    QCategorisedResult& operator=(QCategorisedResult const& other);
    QCategorisedResult(QCategorisedResult&&);
    QCategorisedResult& operator=(QCategorisedResult&&);
    //@}

    /**
    \brief Updates the category of this result.
    \param category The category for the result.
    */
    void set_category(QCategory::SCPtr category);

    /**
    \brief Return category of this result.
    Get the category instance this result belongs to.
    \return The category instance.
    */
    QCategory::SCPtr category() const;

private:
    /// @cond
    internal::QCategorisedResultImpl* fwd() const;
    /// @endcond

    friend class internal::QSearchReplyImpl;
};

}  // namespace qt

}  // namespace scopes

}  // namespace unity

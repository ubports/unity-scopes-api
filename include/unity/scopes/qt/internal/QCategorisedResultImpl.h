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

#include <unity/util/DefinesPtrs.h>

#include <unity/scopes/qt/QCategory.h>

#include <unity/scopes/qt/internal/QResultImpl.h>

#include <QtCore/QString>

namespace unity
{

namespace scopes
{

class CategorisedResult;

namespace internal
{
class ResultImpl;
}

namespace qt
{

namespace internal
{

class QCategorisedResultImpl : public QResultImpl
{
public:
    /// @cond
    UNITY_DEFINES_PTRS(QCategorisedResultImpl);
    /// @endcond
    /**
    \brief Creates a CategorisedResult with given category,
    with all base attributes initially empty.
    \param category The category for the result.
    */
    explicit QCategorisedResultImpl(QCategory::SCPtr category);

    /**@name Copy and assignment
    Copy and assignment operators (move and non-move versions) have the usual value semantics.
    */
    //{@
    QCategorisedResultImpl(QCategorisedResultImpl const& other);
    QCategorisedResultImpl& operator=(QCategorisedResultImpl const& other);
    QCategorisedResultImpl(QCategorisedResultImpl&&);
    QCategorisedResultImpl& operator=(QCategorisedResultImpl&&);
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
    CategorisedResult* api_object() const;
    friend class QSearchReplyImpl;
};

}  // namespace internal

}  // namespace qt

}  // namespace scopes

}  // namespace unity

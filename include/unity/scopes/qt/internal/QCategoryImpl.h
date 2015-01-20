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
#include <unity/util/NonCopyable.h>

#include <unity/scopes/Category.h>

#include <QtCore/QString>
#include <QtCore/QVariantMap>

namespace unity
{

namespace scopes
{

namespace qt
{

namespace internal
{
class QCategorisedResultImpl;

class QCategoryImpl
{
public:
    /// @cond
    NONCOPYABLE(QCategoryImpl);
    UNITY_DEFINES_PTRS(QCategoryImpl);

    QCategoryImpl(unity::scopes::Category::SCPtr category);

    virtual ~QCategoryImpl();
    /// @endcond

    /**
     \brief Get identifier of this Category
     \return The category identifier.
    */
    QString id() const;

    /**
     \brief Get title of this Category
     \return The category title.
    */
    QString title() const;

    /**
     \brief Get icon of this Category
     \return The category icon.
    */
    QString icon() const;

    /**
     \brief Query to perform when this category is expanded
     \return The expansion query or nullptr.
    */
    CannedQuery::SCPtr query() const;

    /**
     \brief Get renderer template of this Category
     \return The category renderer template.
     */
    CategoryRenderer const& renderer_template() const;

    // @cond
    QVariantMap serialize() const;

private:
    unity::scopes::Category::SCPtr api_category_;
    friend class QCategorisedResultImpl;
};

}  // namespace internal

}  // namespace qt

}  // namespace scopes

}  // namespace unity

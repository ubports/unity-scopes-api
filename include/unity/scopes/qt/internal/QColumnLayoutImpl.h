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

#ifndef _ENABLE_QT_EXPERIMENTAL_
#error You should define _ENABLE_QT_EXPERIMENTAL_ in order to use this experimental header file.
#endif

#include <unity/scopes/qt/QColumnLayout.h>

#include <unity/util/DefinesPtrs.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <QtCore/QVariant>
#pragma GCC diagnostic pop
#include <QtCore/QVector>

namespace unity
{

namespace scopes
{

class ColumnLayout;

namespace qt
{

namespace internal
{
class QPreviewReplyImpl;

class QColumnLayoutImpl
{
public:
    /// @cond
    UNITY_DEFINES_PTRS(QColumnLayoutImpl);
    /// @endcond

    /**
    \brief Creates a layout definition that expects num_of_columns columns to be added with ColumnLayout::add_column.
    \param num_of_columns The number of columns for the layout.
    \throws unity::InvalidArgumentException for an invalid number of columns.
    */
    explicit QColumnLayoutImpl(int num_of_columns);

    /**@name Copy and assignment
    Copy and assignment operators (move and non-move versions) have the usual value semantics.
    */
    //{@
    QColumnLayoutImpl(QColumnLayoutImpl const& other);
    QColumnLayoutImpl(QColumnLayoutImpl&&);
    QColumnLayoutImpl& operator=(QColumnLayoutImpl const& other);
    QColumnLayoutImpl& operator=(QColumnLayoutImpl&&);
    //@}

    /// @cond
    ~QColumnLayoutImpl();
    /// @endcond

    /**
    \brief Adds a new column and assigns widgets to it.

    ColumnLayout expects exactly the number of columns passed to the constructor to be created with the
    add_column method.
    \throws unity::LogicException if an attempt is made to add more columns than specified in the constructor.
    \throws unity::LogicException from \link unity::scopes::PreviewReply::register_layout\endlink if a layout
    has fewer columns than specified in the constructor.
    */
    void add_column(QVector<QString> widget_ids);

    /**
    \brief Get the current number of columns in this layout.
    \return The number of columns added with add_column().
    */
    int size() const noexcept;

    /**
    \brief Get the number of columns expected by this layout as specified in the constructor.
    \return The number of columns expected by this layout.
    */
    int number_of_columns() const noexcept;

    /**
    \brief Retrieve the list of widgets for given column.
    \param index The index of a column.
    \return The widget identifiers for the given column index.
    \throws unity::InvalidArgumentException if the index is invalid.
    */
    QVector<QString> column(int index) const;

    /// @cond
    QVariantMap serialize() const;
    /// @endcond

    // added for testing purposes
    static unity::scopes::qt::QColumnLayout create(QColumnLayoutImpl* internal);
    // added for testing purposes
    unity::scopes::ColumnLayout* get_api();

private:
    QColumnLayoutImpl(ColumnLayout* api_layout);
    std::unique_ptr<ColumnLayout> api_layout_;

    friend class unity::scopes::qt::QColumnLayout;
    friend class internal::QPreviewReplyImpl;
};

}  // namespace internal

}  // namespace qt

}  // namespace scopes

}  // namespace unity

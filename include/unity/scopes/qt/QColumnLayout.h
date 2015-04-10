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

namespace qt
{

namespace internal
{
class QColumnLayoutImpl;
class QPreviewReplyImpl;
}

/**
\brief Represents a column layout.

Column layouts are used to represent the view in different ways depending on
the device we are running our application.

If, for example, you run the application in a tablet that has more screen the
application could use more columns than running in a phone.

*/
class QColumnLayout final
{
public:
    /// @cond
    UNITY_DEFINES_PTRS(QColumnLayout);
    /// @endcond

    /**
    \brief Creates a layout definition that expects num_of_columns columns to be added with ColumnLayout::add_column.
    \param num_of_columns The number of columns for the layout.
    \throws unity::InvalidArgumentException for an invalid number of columns.
    */
    explicit QColumnLayout(int num_of_columns);

    /**@name Copy and assignment
    Copy and assignment operators (move and non-move versions) have the usual value semantics.
    */
    //{@
    QColumnLayout(QColumnLayout const& other);
    QColumnLayout(QColumnLayout&&);
    QColumnLayout& operator=(QColumnLayout const& other);
    QColumnLayout& operator=(QColumnLayout&&);
    //@}

    /// @cond
    ~QColumnLayout();
    /// @endcond

    /**
    \brief Adds a new column and assigns widgets to it.

    ColumnLayout expects exactly the number of columns passed to the constructor to be created with the
    add_column method.
    \throws unity::LogicException if an attempt is made to add more columns than specified in the constructor.
    \throws unity::LogicException from \link unity::scopes::qt::QPreviewReply::register_layout\endlink if a layout
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

private:
    QColumnLayout(internal::QColumnLayoutImpl* impl);
    /// @cond
    std::unique_ptr<internal::QColumnLayoutImpl> p;
    friend class internal::QColumnLayoutImpl;
    friend class internal::QPreviewReplyImpl;
    /// @endcond
};

/*! \typedef QColumnLayoutList
\brief List of column layouts (see unity::scopes::qt::QColumnLayout)
*/

typedef QList<QColumnLayout> QColumnLayoutList;

}  // namespace qt

}  // namespace scopes

}  // namespace unity

/*
 * Copyright (C) 2014 Canonical Ltd
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

#ifndef UNITY_SCOPES_COLUMNLAYOUT_H
#define UNITY_SCOPES_COLUMNLAYOUT_H

#include <unity/scopes/Variant.h>
#include <memory>
#include <list>
#include <vector>
#include <unity/util/DefinesPtrs.h>

namespace unity
{

namespace scopes
{

namespace internal
{
class ColumnLayoutImpl;
}

class ColumnLayout final
{
public:
/// @cond
    UNITY_DEFINES_PTRS(ColumnLayout);
/// @endcond

    /**
    \brief Creates a layout definition that expects num_of_columns columns to be added with ColumnLayout::add_column.
    \param The number of columns for the layout.
    \throws unity::InvalidArgumentException for an invalid number of columns.
    */
    explicit ColumnLayout(int num_of_columns);

    /**@name Copy and assignment
    Copy and assignment operators (move and non-move versions) have the usual value semantics.
    */
    //{@
    ColumnLayout(ColumnLayout const& other);
    ColumnLayout(ColumnLayout&&);
    ColumnLayout& operator=(ColumnLayout const& other);
    ColumnLayout& operator=(ColumnLayout&&);
    //@}

    /// @cond
    ~ColumnLayout();
    /// @endcond

    /**
    \brief Adds a new column and assigns widgets to it.

    ColumnLayout expects exactly the number of columns passed to the constructor to be created with the
    add_column method.
    \throws unity::LogicException if an attempt is made to add more columns than specified in the constructor.
    \throws unity::LogicException from \link unity::scopes::PreviewReply::register_layout\endlink if a layout
    has fewer columns than specified in the constructor.
    */
    void add_column(std::vector<std::string> widget_ids);

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
    std::vector<std::string> column(int index) const;

/// @cond
    VariantMap serialize() const;
/// @endcond

private:
    ColumnLayout(internal::ColumnLayoutImpl *impl);
    std::unique_ptr<internal::ColumnLayoutImpl> p;
    friend class internal::ColumnLayoutImpl;
};

/*! \typedef ColumnLayoutList
\brief List of column layouts (see unity::scopes::ColumnLayout)
*/

typedef std::list<ColumnLayout> ColumnLayoutList;

} // namespace scopes

} // namespace unity

#endif

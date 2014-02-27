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
    \brief Creates a layout definition which expects num_of_columns columns to be added with ColumnLayout::add_column.
    The number of columns needs to be greater than 0. Throws unity::InvalidArgumentException on invalid number.
    */
    explicit ColumnLayout(int num_of_columns);

/// @cond
    ColumnLayout(ColumnLayout const& other);
    ColumnLayout(ColumnLayout&&);
    ColumnLayout& operator=(ColumnLayout const& other);
    ColumnLayout& operator=(ColumnLayout&&);
    ~ColumnLayout();
/// @endcond

    /**
    \brief Adds new column and assigns widgets to it.
    ColumnLayout expects exactly the number of columns passed to the constructor to be created with add_column method.
    This method throws unity::LogicException if trying to add more columns than declared at the construction side.
    Failing to add sufficient number of columns will result in unity::LogicException from \link unity::scopes::PreviewReply::register_layout\endlink.
     */
    void add_column(std::vector<std::string> widget_ids);

    /**
    \brief Get real number of columns in this layout.
    Note, this is can be smaller than number of columns passed to the constructor and available via number_of_columns().
    \return The number of columns added with add_column().
     */
    int size() const noexcept;

    /**
    \brief Get number of columns expected by this layout and passed to the constructor.
    \return The number of columns expected by this layout.
    */
    int number_of_columns() const noexcept;

    /**
    \brief Retrieve list of widgets for given column. This can throw unity::InvalidArgumentException if the index is invalid.
    \param index index of a column
    \return The widget identifiers for the given column index.
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

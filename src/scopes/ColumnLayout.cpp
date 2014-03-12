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

#include <unity/scopes/ColumnLayout.h>
#include <unity/scopes/internal/ColumnLayoutImpl.h>

namespace unity
{

namespace scopes
{

/*!
\class ColumnLayout

\brief Defines a layout for preview widgets with given column setup.

ColumnLayout defines how widgets should be laid out on a display with a given number of columns.
In typical use cases, a scope creates ColumnLayout instances for all supported displays (number of columns),
and defines for every instance what widgets belong to which columns. The shell uses the layout
that is most appropriate for the device in use.
Column layout definitions are optional. However, we recommend that scopes define layouts for the best
visual appearance.

An example of creating two layouts, one for a screen with one column and and one for a screen with two columns.

\code{.cpp}
void MyPreview::run(PreviewReplyProxy const& reply)
    // layout definition for a screen with just 1 column
    ColumnLayout layout1col(1);
    layout1col.add_column({"art", "header", "actions"}); // all widgets in a single column

    // layout definition for a screen with 2 columns
    ColumnLayout layout2col(2);
    layout2col.add_column({"art"});  // widget in 1st column
    layout2col.add_column({"header", "actions"}); // widgets in 2nd column

    reply->register_layout({layout1col, layout2col});

    // create widgets
    PreviewWidget w1("art", "image");
    ...
\endcode
*/

ColumnLayout::ColumnLayout(int num_of_columns)
    : p(new internal::ColumnLayoutImpl(num_of_columns))
{
}

/// @cond
ColumnLayout::ColumnLayout(internal::ColumnLayoutImpl *impl)
    : p(impl)
{
}

ColumnLayout::ColumnLayout(ColumnLayout const& other)
    : p(new internal::ColumnLayoutImpl(*(other.p)))
{
}

ColumnLayout::ColumnLayout(ColumnLayout&&) = default;

ColumnLayout& ColumnLayout::operator=(ColumnLayout const& other)
{
    if (this != &other)
    {
        p.reset(new internal::ColumnLayoutImpl(*(other.p)));
    }
    return *this;
}

ColumnLayout::~ColumnLayout() = default;

ColumnLayout& ColumnLayout::operator=(ColumnLayout&&) = default;

VariantMap ColumnLayout::serialize() const
{
    return p->serialize();
}

/// @endcond

void ColumnLayout::add_column(std::vector<std::string> widget_ids)
{
    return p->add_column(widget_ids);
}

int ColumnLayout::size() const noexcept
{
    return p->size();
}

int ColumnLayout::number_of_columns() const noexcept
{
    return p->number_of_columns();
}

std::vector<std::string> ColumnLayout::column(int index) const
{
    return p->column(index);
}
} // namespace scopes

} // namespace unity

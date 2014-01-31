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

ColumnLayout::ColumnLayout(int num_of_columns)
    : p(new internal::ColumnLayoutImpl(num_of_columns))
{
}

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

ColumnLayout& ColumnLayout::operator=(ColumnLayout&&) = default;

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

VariantMap ColumnLayout::serialize() const
{
    return p->serialize();
}

} // namespace scopes

} // namespace unity

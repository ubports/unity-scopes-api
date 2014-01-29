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

#include <unity/scopes/internal/ColumnLayoutImpl.h>
#include <unity/UnityExceptions.h>
#include <sstream>

namespace unity
{

namespace scopes
{

namespace internal
{

ColumnLayoutImpl::ColumnLayoutImpl(unsigned num_of_columns)
    : num_of_columns_(num_of_columns)
{
    if (num_of_columns_ == 0)
    {
        throw unity::InvalidArgumentException("ColumnLayout(): the number of columns must be greater than 0");
    }
}

void ColumnLayoutImpl::add_column(std::vector<std::string> widget_ids)
{
    if (columns_.size() > num_of_columns_)
    {
        std::ostringstream str;
        str << "ColumnLayout::add_column(): excessive column, exepcting " << num_of_columns_ << " columns";
        throw unity::LogicException(str.str());
    }
    columns_.push_back(widget_ids);
}

unsigned ColumnLayoutImpl::size() const noexcept
{
    return columns_.size();
}

unsigned ColumnLayoutImpl::number_of_columns() const noexcept
{
    return num_of_columns_;
}

std::vector<std::string> ColumnLayoutImpl::column(unsigned index) const
{
    if (index >= columns_.size())
    {
        throw unity::InvalidArgumentException("ColumnLayout::column(): invalid column index");
    }
    return columns_[index];
}

VariantMap ColumnLayoutImpl::serialize() const
{
    //TODO
}

} // namespace internal

} // namespace scopes

} // namespace unity

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
#include <unordered_set>
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

ColumnLayoutImpl::ColumnLayoutImpl(VariantMap const& var)
{
    auto it = var.find("column_data");
    if (it == var.end())
    {
        throw unity::InvalidArgumentException("ColumnLayoutImpl(): missing column_data");
    }

    auto const& outerarr = it->second.get_array();
    num_of_columns_ = outerarr.size();
    for (auto const& arr: outerarr)
    {
        std::vector<std::string> widgets;
        for (auto const& w: arr.get_array())
        {
            widgets.push_back(w.get_string());
        }
        columns_.push_back(widgets);
    }
}

void ColumnLayoutImpl::add_column(std::vector<std::string> widget_ids)
{
    if (columns_.size() >= num_of_columns_)
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
    VariantArray outerarr;
    for (auto const& col: columns_)
    {
        VariantArray arr;
        for (auto const& w: col)
        {
            arr.push_back(Variant(w));
        }
        outerarr.push_back(Variant(arr));
    }
    VariantMap vm;
    vm["column_data"] = Variant(outerarr);
    return vm;
}

ColumnLayout ColumnLayoutImpl::create(VariantMap const& var)
{
    return ColumnLayout(new ColumnLayoutImpl(var));
}

void ColumnLayoutImpl::validate_layouts(ColumnLayoutList const& layouts)
{
    std::unordered_set<unsigned> layout_number_lut;

    // basic check for consistency of layouts
    for (auto const& layout: layouts)
    {
        if (layout.size() != layout.number_of_columns())
        {
            std::ostringstream str;
            str << "ColumnLayout::validate_layouts(): expected " << layout.number_of_columns() << " but only " << layout.size() << " defined";
            throw LogicException(str.str());
        }
        if (layout_number_lut.find(layout.number_of_columns()) != layout_number_lut.end())
        {
            std::ostringstream str;
            str << "ColumnLayout::validate_layouts(): duplicate definition of layout with " << layout.number_of_columns() << " number of columns";
            throw LogicException(str.str());
        }
        layout_number_lut.insert(layout.size());
    }
}

} // namespace internal

} // namespace scopes

} // namespace unity

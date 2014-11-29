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

#pragma once

#include <unity/scopes/ColumnLayout.h>
#include <unity/scopes/Variant.h>
#include <string>
#include <vector>

namespace unity
{

namespace scopes
{

namespace internal
{

class ColumnLayoutImpl
{
public:
    explicit ColumnLayoutImpl(int num_of_columns);
    explicit ColumnLayoutImpl(VariantMap const& var);
    ColumnLayoutImpl(ColumnLayoutImpl const& other) = default;
    void add_column(std::vector<std::string> widget_ids);
    int size() const noexcept;
    int number_of_columns() const noexcept;
    std::vector<std::string> column(int index) const;
    VariantMap serialize() const;

    static ColumnLayout create(VariantMap const& var);
    static void validate_layouts(ColumnLayoutList const& layouts);

private:
    int num_of_columns_;
    std::vector<std::vector<std::string>> columns_;
    static const int max_number_of_columns_ = 1024;
};

} // namespace internal

} // namespace scopes

} // namespace unity

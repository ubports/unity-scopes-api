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

#ifndef UNITY_INTERNAL_COLUMNLAYOUTIMPL_H
#define UNITY_INTERNAL_COLUMNLAYOUTIMPL_H

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
    explicit ColumnLayoutImpl(unsigned num_of_columns);
    explicit ColumnLayoutImpl(VariantMap const& var);
    void add_column(std::vector<std::string> widget_ids);
    unsigned size() const noexcept;
    unsigned number_of_columns() const noexcept;
    std::vector<std::string> column(unsigned index) const;
    VariantMap serialize() const;

    static ColumnLayout create(VariantMap const& var);
    static void validate_layouts(ColumnLayoutList const& layouts);

private:
    unsigned num_of_columns_;
    std::vector<std::vector<std::string>> columns_;
};

} // namespace internal

} // namespace scopes

} // namespace unity

#endif

/*
 * Copyright (C) 2013 Canonical Ltd
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

#ifndef UNITY_INTERNAL_PREVIEWWIDGETDEFINITIONBUILDER_H
#define UNITY_INTERNAL_PREVIEWWIDGETDEFINITIONBUILDER_H

#include <unity/scopes/Variant.h>
#include <string>
#include <tuple>

namespace unity
{

namespace scopes
{

namespace internal
{

class VariantMapBuilderImpl final
{
public:
    VariantMapBuilderImpl() = default;
    ~VariantMapBuilderImpl() = default;
    void add_attribute(std::string const& key, Variant const& value);
    void add_tuple(std::string const& array_key, std::initializer_list<std::pair<std::string, Variant>> const& tuple);
    void add_tuple(std::string const& array_key, std::vector<std::pair<std::string, Variant>> const& tuple);
    VariantMap variant_map() const;

private:
    VariantMap variant_;
};

} // namespace internal

} // namespace scopes

} // namespace unity

#endif

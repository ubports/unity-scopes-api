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

#ifndef UNITY_INTERNAL_VARIANTBUILDER_H
#define UNITY_INTERNAL_VARIANTBUILDER_H

#include <unity/scopes/Variant.h>
#include <string>
#include <tuple>

namespace unity
{

namespace scopes
{

namespace internal
{

class VariantBuilderImpl final
{
public:
    VariantBuilderImpl() = default;
    VariantBuilderImpl(VariantBuilderImpl const& other) = default;
    ~VariantBuilderImpl() = default;
    void add_tuple(std::initializer_list<std::pair<std::string, Variant>> const& tuple);
    void add_tuple(std::vector<std::pair<std::string, Variant>> const& tuple);
    VariantArray to_variant_array() const;

private:
    Variant variant_;
};

} // namespace internal

} // namespace scopes

} // namespace unity

#endif

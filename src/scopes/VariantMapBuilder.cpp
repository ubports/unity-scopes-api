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

#include <unity/scopes/VariantMapBuilder.h>
#include <unity/scopes/internal/VariantMapBuilderImpl.h>

namespace unity
{

namespace scopes
{

VariantMapBuilder::VariantMapBuilder() = default;

VariantMapBuilder::~VariantMapBuilder() = default;

void VariantMapBuilder::add_attribute(std::string const& key, Variant const& value)
{
    p->add_attribute(key, value);
}

void VariantMapBuilder::add_tuple(std::string const& array_key, std::initializer_list<std::pair<std::string, Variant>> const& tuple)
{
    p->add_tuple(array_key, tuple);
}

} // namespace scopes

} // namespace unity

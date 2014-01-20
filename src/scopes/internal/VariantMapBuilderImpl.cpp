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

#include <unity/scopes/internal/VariantMapBuilderImpl.h>

namespace unity
{

namespace scopes
{

namespace internal
{

void VariantMapBuilderImpl::add_attribute(std::string const& key, Variant const& value)
{
    variant_[key] = value;
}

void VariantMapBuilderImpl::add_tuple(std::string const& array_key, std::initializer_list<std::pair<std::string, Variant>> const& tuple)
{
    VariantArray va;
    auto it = variant_.find(array_key);
    if (it != variant_.end())
    {
        va = it->second.get_array();
    }

    va.push_back(Variant(VariantMap(tuple.begin(), tuple.end())));
    variant_[array_key] = std::move(va);
}

void VariantMapBuilderImpl::add_tuple(std::string const& array_key, std::vector<std::pair<std::string, Variant>> const& tuple)
{
    VariantArray va;
    auto it = variant_.find(array_key);
    if (it != variant_.end())
    {
        va = it->second.get_array();
    }

    va.push_back(Variant(VariantMap(tuple.begin(), tuple.end())));
    variant_[array_key] = std::move(va);
}

VariantMap VariantMapBuilderImpl::variant_map() const
{
    return variant_;
}

} // namespace internal

} // namespace scopes

} // namespace unity

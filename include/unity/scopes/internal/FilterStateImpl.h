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

#ifndef UNITY_INTERNAL_FILTERSTATEIMPL_H
#define UNITY_INTERNAL_FILTERSTATEIMPL_H

#include <unity/scopes/Variant.h>
#include <unity/scopes/FilterState.h>
#include <string>

namespace unity
{

namespace scopes
{

namespace internal
{

class FilterStateImpl final
{
public:
    FilterStateImpl() = default;
    FilterStateImpl(VariantMap const& var);
    ~FilterStateImpl() = default;
    FilterStateImpl(FilterStateImpl const& other) = default;

    bool has_filter(std::string const& id) const;
    void remove(std::string const& id);
    Variant get(std::string const& filter_id) const;
    VariantMap& get();
    VariantMap serialize() const;
    static FilterState deserialize(VariantMap const& var);

private:
    VariantMap state_;
};

} // namespace internal

} // namespace scopes

} // namespace unity

#endif

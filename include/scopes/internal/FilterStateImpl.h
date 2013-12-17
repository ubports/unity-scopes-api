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

#include <scopes/Variant.h>
#include <string>

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

class FilterStateImpl final
{
public:
    FilterStateImpl() = default;
    ~FilterStateImpl() = default;
    FilterStateImpl(FilterStateImpl const& other) = default;

    bool has_filter(std::string const& id);
    void reset(std::string const& id);

    // store (filter_id, [...]) mapping; mapped value is a VariantArray of active option ids
    void set_option_selector_value(std::string const& filter_id, std::string const& option_id, bool value);

private:
    VariantMap state_;
};

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

#endif

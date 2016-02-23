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

#include <unity/scopes/FilterOption.h>
#include <unity/scopes/internal/FilterOptionImpl.h>

namespace unity
{

namespace scopes
{

FilterOption::FilterOption(std::string const& id, std::string const& label)
    : p(new internal::FilterOptionImpl(id, label))
{
}

/// @cond
FilterOption::~FilterOption() = default;
/// @endcond

std::string FilterOption::id() const
{
    return p->id();
}

std::string FilterOption::label() const
{
    return p->label();
}

bool FilterOption::default_value() const
{
    return p->default_value();
}

} // namespace scopes

} // namespace unity

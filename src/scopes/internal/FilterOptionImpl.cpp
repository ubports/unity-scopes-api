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

#include <unity/scopes/internal/FilterOptionImpl.h>

#include <unity/UnityExceptions.h>

namespace unity
{

namespace scopes
{

namespace internal
{

FilterOptionImpl::FilterOptionImpl(std::string const& id, std::string const& label)
    : id_(id),
      label_(label),
      default_value_(false)
{
    if (id_.empty())
    {
        throw InvalidArgumentException("FilterOption(): invalid empty id string");
    }
    if (label_.empty())
    {
        throw InvalidArgumentException("FilterOption(): invalid empty label string");
    }
}

std::string FilterOptionImpl::id() const
{
    return id_;
}

std::string FilterOptionImpl::label() const
{
    return label_;
}

void FilterOptionImpl::set_default_value(bool default_value)
{
    default_value_ = default_value;
}

bool FilterOptionImpl::default_value() const
{
    return default_value_;
}

} // namespace internal

} // namespace scopes

} // namespace unity

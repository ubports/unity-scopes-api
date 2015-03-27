/*
 * Copyright (C) 2015 Canonical Ltd
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

#include <unity/scopes/internal/DateTimePickerFilterImpl.h>

namespace unity
{

namespace scopes
{

namespace internal
{

DateTimePickerFilterImpl::DateTimePickerFilterImpl(std::string const& id, DateTimePickerFilter::Mode mode)
    : FilterBaseImpl(id)
{
}

void DateTimePickerFilterImpl::set_time_label(std::string const& label)
{
}

void DateTimePickerFilterImpl::set_date_label(std::string const& label)
{
}

void DateTimePickerFilterImpl::set_minimum(std::chrono::system_clock::time_point const& date)
{
}

void DateTimePickerFilterImpl::set_maximum(std::chrono::system_clock::time_point const& date)
{
}

std::string DateTimePickerFilterImpl::time_label() const
{
    return "";
}

std::string DateTimePickerFilterImpl::date_label() const
{
    return "";
}

std::chrono::system_clock::time_point DateTimePickerFilterImpl::minimum() const
{
    return std::chrono::system_clock::time_point::min();
}

std::chrono::system_clock::time_point DateTimePickerFilterImpl::maximum() const
{
    return std::chrono::system_clock::time_point::min();
}

bool DateTimePickerFilterImpl::has_selected_date(FilterState const& filter_state) const
{
    return false;
}

std::chrono::system_clock::time_point DateTimePickerFilterImpl::selected_date(FilterState const& filter_state) const
{
    return std::chrono::system_clock::time_point::min();
}

void DateTimePickerFilterImpl::update_state(FilterState& filter_state, std::chrono::system_clock::time_point const& date) const
{
}

void DateTimePickerFilterImpl::update_state(FilterState& filter_state, std::string const& filter_id, std::chrono::system_clock::time_point const& date)
{
}

void DateTimePickerFilterImpl::serialize(VariantMap& var) const
{
}

void DateTimePickerFilterImpl::deserialize(VariantMap const& var)
{
}

std::string DateTimePickerFilterImpl::filter_type() const
{
    return "";
}

}

}

}


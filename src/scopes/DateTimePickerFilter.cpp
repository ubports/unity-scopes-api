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

#include <unity/scopes/DateTimePickerFilter.h>
#include <unity/scopes/internal/DateTimePickerFilterImpl.h>

namespace unity
{

namespace scopes
{

DateTimePickerFilter::DateTimePickerFilter(internal::DateTimePickerFilterImpl* impl)
    : FilterBase(impl)
{
}

DateTimePickerFilter::UPtr DateTimePickerFilter::create(std::string const& id, DateTimePickerFilter::Mode mode)
{
    return std::unique_ptr<DateTimePickerFilter>(new DateTimePickerFilter(new internal::DateTimePickerFilterImpl(id, mode)));
}

void DateTimePickerFilter::set_time_label(std::string const& label)
{
    fwd()->set_time_label(label);
}

void DateTimePickerFilter::set_date_label(std::string const& label)
{
    fwd()->set_date_label(label);
}

void DateTimePickerFilter::set_maximum(std::chrono::system_clock::time_point const& date)
{
    fwd()->set_maximum(date);
}

void DateTimePickerFilter::set_minimum(std::chrono::system_clock::time_point const& date)
{
    fwd()->set_minimum(date);
}

std::string DateTimePickerFilter::time_label() const
{
    return fwd()->time_label();
}

std::string DateTimePickerFilter::date_label() const
{
    return fwd()->date_label();
}

DateTimePickerFilter::Mode DateTimePickerFilter::mode() const
{
    return fwd()->mode();
}

bool DateTimePickerFilter::has_minimum() const
{
    return fwd()->has_minimum();
}

bool DateTimePickerFilter::has_maximum() const
{
    return fwd()->has_maximum();
}

std::chrono::system_clock::time_point DateTimePickerFilter::minimum() const
{
    return fwd()->minimum();
}

std::chrono::system_clock::time_point DateTimePickerFilter::maximum() const
{
    return fwd()->maximum();
}

bool DateTimePickerFilter::has_selected_date(FilterState const& filter_state) const
{
    return fwd()->has_selected_date(filter_state);
}

std::chrono::system_clock::time_point DateTimePickerFilter::selected_date(FilterState const& filter_state) const
{
    return fwd()->selected_date(filter_state);
}

void DateTimePickerFilter::update_state(FilterState& filter_state, std::chrono::system_clock::time_point const& date) const
{
    fwd()->update_state(filter_state, date);
}

void DateTimePickerFilter::update_state(FilterState& filter_state, std::string const& filter_id, std::chrono::system_clock::time_point const& date)
{
    internal::DateTimePickerFilterImpl::update_state(filter_state, filter_id, date);
}

internal::DateTimePickerFilterImpl* DateTimePickerFilter::fwd() const
{
    return dynamic_cast<internal::DateTimePickerFilterImpl*>(p.get());
}

}

}

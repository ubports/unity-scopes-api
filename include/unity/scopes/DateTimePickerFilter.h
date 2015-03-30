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

#pragma once

#include <unity/scopes/FilterBase.h>
#include <chrono>
#include <bitset>

namespace unity
{

namespace scopes
{
class FilterState;

namespace internal
{
class DateTimePickerFilterImpl;
}

/**
\brief TODO
*/

class DateTimePickerFilter : public FilterBase
{
public:
/// @cond
    UNITY_DEFINES_PTRS(DateTimePickerFilter);
/// @endcond

    enum ModeFLags {
        Years,
        Months,
        Days,
        Hours,
        Minutes,
        Seconds
    };

    typedef std::bitset<6> Mode;

    /**
    */
    static DateTimePickerFilter::UPtr create(std::string const& id, Mode mode);

    void set_time_label(std::string const& label);
    void set_date_label(std::string const& label);

    void set_minimum(std::chrono::system_clock::time_point const& date);
    void set_maximum(std::chrono::system_clock::time_point const& date);

    /**
    */
    std::string time_label() const;
    std::string date_label() const;

    std::chrono::system_clock::time_point minimum() const;
    std::chrono::system_clock::time_point maximum() const;

    bool has_selected_date(FilterState const& filter_state) const;

    /**
    \throws unity::scopes::NotFoundException if date is not set.
    */
    std::chrono::system_clock::time_point selected_date(FilterState const& filter_state) const;

    /**
    */
    void update_state(FilterState& filter_state, std::chrono::system_clock::time_point const& date) const;

    /**
    */
    static void update_state(FilterState& filter_state, std::string const& filter_id, std::chrono::system_clock::time_point const& date);

private:
    DateTimePickerFilter(internal::DateTimePickerFilterImpl*);
    internal::DateTimePickerFilterImpl* fwd() const;
    friend class internal::DateTimePickerFilterImpl;
};

} // namespace scopes

} // namespace unity

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

namespace experimental
{

/**
\brief A date and/or time picker filter.

The DateTimePickerFilter displays a date and/or time picker widget, that can optionally be
constrained by minimum and maximum point in time.
*/

class DateTimePickerFilter : public FilterBase
{
public:
/// @cond
    UNITY_DEFINES_PTRS(DateTimePickerFilter);
/// @endcond

    /**
     \brief Flags that specify what kind of value selectors should be shown by the picker.

     These correspond to the "mode" flags of Ubuntu SDK's DatePicker QML widget. However, unlike the QML widget,
     DateTimePickerFilter's mode can combine both date and time values, in which case two DatePicker widgets will
     be displayed.
     */
    enum ModeFlags {
        Years,
        Months,
        Days,
        Hours,
        Minutes,
        Seconds
    };

    /**
     \brief A combination of ModeFlags
    */
    typedef std::bitset<6> Mode;

    /**
     \brief Create DateTimePickerFilter.

     Creates DateTimePickerFilter with given id and mode.

     \param id A unique identifier for the filter that can be used to later identify it among several filters.
     \param mode Mode flags that configure values displayed by the filter in the UI.
    */
    static DateTimePickerFilter::UPtr create(std::string const& id, Mode mode);

    /**
     \brief Sets optional label for the time widget.

     Sets the label to be displayed alongside time widget. Note that whether the label is displayed
     depends on the Mode flags.
     */
    void set_time_label(std::string const& label);

    /**
     \brief Sets optional label for the date widget.

     Sets the label to be displayed alongside date widget. Note that whether the label is displayed
     depends on the Mode flags.
     */
    void set_date_label(std::string const& label);

    /**
     \brief Sets optional minimum allowed date.

     Sets the minimum date allowed by this filter. This will constrain values displayed by the UI
     and passed to the scope.
     */
    void set_minimum(std::chrono::system_clock::time_point const& date);

    /**
     \brief Sets optional maximum allowed date.

     Sets the minimum date allowed by this filter. This will constrain values displayed by the UI
     and passed to the scope.
     */
    void set_maximum(std::chrono::system_clock::time_point const& date);

    /**
     \brief Get label of the time widget.

     \return The label
    */
    std::string time_label() const;

    /**
     \brief Get label of the date widget.

     \return The label
    */
    std::string date_label() const;

    /**
     \brief Get mode of this filter.

     \return The filter mode
     */
    Mode mode() const;

    /**
     \brief Checks if a minimum date has been set for this filter.

     \return true if minimum date has been set
     */
    bool has_minimum() const;

    /**
     \brief Checks if a maximum date has been set for this filter.

     \return true if maximum date has been set
    */
    bool has_maximum() const;

    /**
     \brief Get minimum date set for this filter.

     \return The minimum date
     \throws unity::LogicException if minimum date hasn't been set
     */
    std::chrono::system_clock::time_point minimum() const;

    /**
     \brief Get maximum date set for this filter.

     \return The maximum date
     \throws unity::LogicException if maximum date hasn't been set
     */
    std::chrono::system_clock::time_point maximum() const;

    /**
    \brief Check if filter state object holds a date set for this filter.

    \return true if filter_state has a date for this filter.
     */
    bool has_selected_date(FilterState const& filter_state) const;

    /**
     \brief Get selected date.

     \return The selected date
     \throws unity::scopes::NotFoundException if date is not set.
     \throws unity::LogicException if selected date is invalid (e.g. out of minimum and maximum range)
    */
    std::chrono::system_clock::time_point selected_date(FilterState const& filter_state) const;

    /**
     \brief Sets selected date for this filter in the filter state object.

     \throws unity::LogicException if date is out of (minimum, maximum) range (as long as they were set).
    */
    void update_state(FilterState& filter_state, std::chrono::system_clock::time_point const& date) const;

    /**
     \brief Sets selected date for this filter in the filter state object, without having an instance of DateTimePickerFilter.

     Updates an instance of FilterState, without the need for a DateTimePickerFilter instance. This is meant
     to be used when creating a canned Query that references another scope.
     Note: this method doesn't cannot validate date against minimum / maximum dates the filter may expect; the date will be
     validated in DateTimePickerFilter::selected_date(FilterState const&) when received by the scope.
    */
    static void update_state(FilterState& filter_state, std::string const& filter_id, std::chrono::system_clock::time_point const& date);

private:
    DateTimePickerFilter(internal::DateTimePickerFilterImpl*);
    internal::DateTimePickerFilterImpl* fwd() const;
    friend class internal::DateTimePickerFilterImpl;
};

} // namespace experimental

} // namespace scopes

} // namespace unity

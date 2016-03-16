/*
 * Copyright (C) 2014 Canonical Ltd
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
#include <unity/scopes/FilterGroup.h>

namespace unity
{

namespace scopes
{

class FilterState;

namespace internal
{
class ValueSliderFilterImpl;
}

class ValueSliderLabels;

/**
\brief A value slider filter that allows for selecting a value within a given range.

The ValueSliderFilter allows for selecting a value within a range defined by minimum and maximum values.
Both minimum and maximum values can have labels and, in addition, the scope may provide extra labels
to mark other values from that range - these label will serve as a guidance to the user.
*/
class UNITY_API ValueSliderFilter : public FilterBase
{
public:
    /// @cond
    UNITY_DEFINES_PTRS(ValueSliderFilter);
    /// @endcond

    /**
     \brief Create ValueSliderFilter.

     Creates ValueSliderFilter of ValueSliderFilter::SliderType::LessThan type.

     \param id A unique identifier for the filter that can be used to later identify it among several filters.
     \param min The minimum allowed value
     \param max The maximum allowed value
     \param default_value The default value of this filter, from the min..max range.
     \param value_labels The labels for min and max values as well as optional extra labels.
     \return Instance of ValueSliderFilter.
     \throws unity::LogicException on invalid (min, max) range or erroneous value_labels.
    */
    static ValueSliderFilter::UPtr create(std::string const& id, double min, double max, double default_value, ValueSliderLabels const& value_labels);

    /**
     \brief Create ValueSliderFilter inside a FilterGroup.

     Creates ValueSliderFilter of ValueSliderFilter::SliderType::LessThan type.

     \param id A unique identifier for the filter that can be used to later identify it among several filters.
     \param min The minimum allowed value
     \param max The maximum allowed value
     \param default_value The default value of this filter, from the min..max range.
     \param value_labels The labels for min and max values as well as optional extra labels.
     \param group A filter group this filter should be added to.
     \return Instance of ValueSliderFilter.
     \throws unity::LogicException on invalid (min, max) range or erroneous value_labels.
    */
    static ValueSliderFilter::UPtr create(std::string const& id, double min, double max, double default_value, ValueSliderLabels const& value_labels, FilterGroup::SCPtr const& group);

    /**
     \brief Change the default value of this filter.

     The default value will be used by the shell if no value is present for this filter in the
     unity::scopes::FilterState (no value has been set by the user).

     \param val The new default value.
    */
    void set_default_value(double val);

    /**
     \brief Get the default value of this filter.

     The default value will be used by the shell if no value is present for this filter in the
     unity::scopes::FilterState (no value has been set by the user).

     \return The default value
    */
    double default_value() const;

    /**
     \brief Get the minimum allowed value.

     \return mimimum value
    */
    double min() const;

    /**
    \brief Get the maximum allowed value.

    \return maximum value
    */
    double max() const;

    /**
    \brief Check if filter state object holds a value of this filter.

    \return true if filter_state has a value of this filter.
     */
    bool has_value(FilterState const& filter_state) const;

    /**
     \brief Get value of this filter from filter state object.

     \return value of this filter or the default value if value is not present in the filter_state.
    */
    double value(FilterState const& filter_state) const;

    /**
     \brief Get value labeles for this slider filter.

     \return the value labels.
     */
    ValueSliderLabels const& labels() const;

    /**
    \brief Sets value of this filter instance in filter state object.

    This is meant to be used to modify a FilterState received with a search request before sending it back to the client (UI shell).

    \throws unity::LogicException if value is out of (min, max) range.
    */
    void update_state(FilterState& filter_state, double value) const;

    /**
    \brief Sets value of this filter instance in filter state object, without having an instance of ValueSliderFilter.

    Updates an instance of FilterState, without the need for an ValueSliderFilter instance. This is meant
    to be used when creating a canned Query that references another scope.
    */
    static void update_state(FilterState& filter_state, std::string const& filter_id, double value);

private:
    ValueSliderFilter(internal::ValueSliderFilterImpl*);
    internal::ValueSliderFilterImpl* fwd() const;
    friend class internal::ValueSliderFilterImpl;
};

} // namespace scopes

} // namespace unity

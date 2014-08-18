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

#ifndef UNITY_SCOPES_VALUESLIDERFILTER_H
#define UNITY_SCOPES_VALUESLIDERFILTER_H

#include <unity/scopes/FilterBase.h>

namespace unity
{

namespace scopes
{

class FilterState;

namespace internal
{
class ValueSliderFilterImpl;
}

namespace experimental
{

/**
\brief A value slider filter that allows for selecting a value within given range.

The ValueSliderFilter displays a static label, such as "Maximum size" and a slider that
allows for selecting a value within range defined by minimum and maximum values. The currently
selected value gets displayed using a label template provided by scope, e.g. "Less than %1 MB",
resulting in labels such as "Less than 40.5 MB".
*/
class UNITY_API ValueSliderFilter : public FilterBase
{
public:
    /// @cond
    UNITY_DEFINES_PTRS(ValueSliderFilter);
    /// @endcond

    /**
     \brief The type of value slider.

     The type should be set according to the behaviour of the filter:
     ValueSliderFilter::SliderType::LessThan should be used if values lesser than selected value are to be returned, otherwise
     ValueSliderFilter::SliderType::MoreThan should be used.
     Note: the selected type is a display hint for the Shell only. It's up to the scope to set it properly and actually follow
     it on search.
     */
    enum SliderType
    {
        LessThan, /**< Scope will search for values lesser than selected value */
        MoreThan  /**< Scope will search for values greater than selected value */
    };

    /**
     \brief Create ValueSliderFilter.

     Creates ValueSliderFilter of ValueSliderFilter::SliderType::LessThan type.

     \param id A unique identifier for the filter that can be used to identify it later among several filters.
     \param label The label for the filter
     \param label_template The template for value label, e.g. "Less than %1"
     \param min The minimum allowed value
     \param max The maximum allowed value
     \return Instance of ValueSliderFilter.
     \throws unity::LogicException on invalid (min, max) range.
    */
    static ValueSliderFilter::UPtr create(std::string const& id, std::string const& label, std::string const& label_template, double min, double max);

    /**
     \brief Change the type of this filter.

     \param tp The type of slider filter.
     */
    void set_slider_type(SliderType tp);

    /**
     \brief Change the default value of this filter.

     The default value of this filter is by default the maximum value allowed. This value is used when calling unity::scopes::ValueSliderFilter::value_label()
     and there is no state for this filter in unity::scopes::FilterState.

     \param val The new default value.
    */
    void set_default_value(double val);

    /**
     \brief Get the type of this filter.

     \return The type of slider filter.
     */
    SliderType slider_type() const;

    /**
     \brief Get the default value of this filter.

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
    \brief Get the label of this filter.

    \return The label
    */
    std::string label() const;

    /**
     \brief Get the value label template of this filter.

     \return The value label template.
     */
    std::string value_label_template() const;

    /**
    \brief Check if filter state object holds a value of this filter.

    \return true if filter_state has a value of this filter.
     */
    bool has_value(FilterState const& filter_state) const;

    /**
     \brief Get value of this filter from filter state object.

     \return value of this filter
     \throws unity::scopes::NotFoundException if value is not present in state object.
    */
    double value(FilterState const& filter_state) const;

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

} // namespace experimental

} // namespace scopes

} // namespace unity

#endif

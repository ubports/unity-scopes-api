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
#include <unity/scopes/Variant.h>
#include <unity/scopes/FilterGroup.h>

namespace unity
{

namespace scopes
{

class FilterState;

namespace internal
{
class RangeInputFilterImpl;
}

/**
\brief A range filter which allows a start and end value to be entered by user, and any of them is optional.

RangeInputFilter is composed of two input boxes which accept numeric values. Any of them is optional. It can have
up to five labels: two labels for start value (prefix or postfix label, displayed left or right to the input box, respectively),
two labels for end value and a single label displayed in between both input boxes.
The filter may provide defaults for start and end values. Default values are used by the UI if user didn't enter values.
*/
class UNITY_API RangeInputFilter : public FilterBase
{
public:
/// @cond
    UNITY_DEFINES_PTRS(RangeInputFilter);
/// @endcond

    /**
    \brief Creates a RangeInputFilter.
    \param id A unique identifier for the filter that can be used to identify it later among several filters.
    \param start_prefix_label A display label for the input box of start value, displayed to the left (can be empty).
    \param start_postfix_label A display label for the input box of start value, displayed to the right (can be empty).
    \param central_label A display label displayed between the two input boxes (can be empty).
    \param end_prefix_label A display label for the input box of end value, displayed to the left (can be empty).
    \param end_postfix_label A display label for the input box of end value, displayed to the right (can be empty).
    \return Instance of RangeInputFilter.
    */
    static RangeInputFilter::SPtr create(std::string const& id,
            std::string const& start_prefix_label, std::string const& start_postfix_label,
            std::string const& central_label,
            std::string const& end_prefix_label, std::string const& end_postfix_label);

    /**
    \brief Creates a RangeInputFilter with specific default values for start and end.
    \param id A unique identifier for the filter that can be used to identify it later among several filters.
    \param default_start_value A default start value (double, int or null).
    \param default_end_value A default end value (double, int or null).
    \param start_prefix_label A display label for the input box of start value, displayed to the left (can be empty).
    \param start_postfix_label A display label for the input box of start value, displayed to the right (can be empty).
    \param central_label A display label displayed between the two input boxes (can be empty).
    \param end_prefix_label A display label for the input box of end value, displayed to the left (can be empty).
    \param end_postfix_label A display label for the input box of end value, displayed to the right (can be empty).
    \return Instance of RangeInputFilter.
    */
    static RangeInputFilter::SPtr create(std::string const& id,
            Variant const& default_start_value,
            Variant const& default_end_value,
            std::string const& start_prefix_label, std::string const& start_postfix_label,
            std::string const& central_label,
            std::string const& end_prefix_label, std::string const& end_postfix_label);

    /**
    \brief Creates a RangeInputFilter inside a FilterGroup.
    \param id A unique identifier for the filter that can be used to identify it later among several filters.
    \param start_prefix_label A display label for the input box of start value, displayed to the left (can be empty).
    \param start_postfix_label A display label for the input box of start value, displayed to the right (can be empty).
    \param central_label A display label displayed between the two input boxes (can be empty).
    \param end_prefix_label A display label for the input box of end value, displayed to the left (can be empty).
    \param end_postfix_label A display label for the input box of end value, displayed to the right (can be empty).
    \param group A filter group this filter should be added to.
    \return Instance of RangeInputFilter.
    */
    static RangeInputFilter::SPtr create(std::string const& id,
            std::string const& start_prefix_label, std::string const& start_postfix_label,
            std::string const& central_label,
            std::string const& end_prefix_label, std::string const& end_postfix_label, FilterGroup::SCPtr const& group);

    /**
    \brief Creates a RangeInputFilter with specific default values for start and end, inside a FilterGroup.
    \param id A unique identifier for the filter that can be used to identify it later among several filters.
    \param start_prefix_label A display label for the input box of start value, displayed to the left (can be empty).
    \param start_postfix_label A display label for the input box of start value, displayed to the right (can be empty).
    \param central_label A display label displayed between the two input boxes (can be empty).
    \param end_prefix_label A display label for the input box of end value, displayed to the left (can be empty).
    \param end_postfix_label A display label for the input box of end value, displayed to the right (can be empty).
    \param group A filter group this filter should be added to.
    \return Instance of RangeInputFilter.
    */
    static RangeInputFilter::SPtr create(std::string const& id,
            Variant const& default_start_value,
            Variant const& default_end_value,
            std::string const& start_prefix_label, std::string const& start_postfix_label,
            std::string const& central_label,
            std::string const& end_prefix_label, std::string const& end_postfix_label,
            FilterGroup::SCPtr const& group);

    /**
    \brief Get the prefix label of start value.
    \return The prefix label of start value.
    */
    std::string start_prefix_label() const;

    /**
    \brief Get the postfix label of start value.
    \return The postfix label of start value.
    */
    std::string start_postfix_label() const;

    /**
    \brief Get the prefix label of end value.
    \return The label of end value.
    */
    std::string end_prefix_label() const;

    /**
    \brief Get the postfix label of end value.
    \return The label of end value.
    */
    std::string end_postfix_label() const;

    /**
    \brief Get the central label for this filter.
    \return The central label.
    */
    std::string central_label() const;

    /**
    \brief Get default start value if set (int, double or null).

    The default value should be used unless a value is present in the FilterState -
    use unity::scopes::RangeInputFilter::has_start_value() and unity::scopes::RangeInputFilter::start_value()
    before resorting to the default value.
    */
    Variant default_start_value() const;

    /**
    \brief Get default end value if set (int, double or null).

    The default value should be used unless a value is present in the FilterState -
    use unity::scopes::RangeInputFilter::has_end_value() and unity::scopes::RangeInputFilter::end_value()
    before resorting to the default value.
    */
    Variant default_end_value() const;

    /**
    \brief Check if filter state holds a start value for this filter instance.

    \param filter_state The state of filters.
    \return true if filter_state has a start value for this filter.
     */
    bool has_start_value(FilterState const& filter_state) const;

    /**
    \brief Check if filter state holds an end value for this filter instance.

    \param filter_state The state of filters.
    \return true if filter_state has an end value for this filter.
     */
    bool has_end_value(FilterState const& filter_state) const;

    /**
     \brief Get start value for this filter instance.

     \return The start value.
     \throws unity::scopes::NotFoundException if start value is not set.
     */
    double start_value(FilterState const& filter_state) const;

    /**
     \brief Get end value for this filter instance.

     \return The end value.
     \throws unity::scopes::NotFoundException if end value is not set.
     */
    double end_value(FilterState const& filter_state) const;

    /**
     \brief Store start and end value for this filter in the filter state.

     Updates filter_state with start and end values for this filter instance. Allowed data types for start_value and end_value
     are Variant::Type::Null, Variant::Type::Double and Variant::Type::Int. Integer values will get converted to double when
     returned via unity::scopes::RangeInputFilter::start_value() and unity::scopes::RangeInputFilter::end_value() methods.
     Pass Variant::null() as start_value or end_value if that value is unspecified (hasn't been entered or got erased by the user
     from the input box).

     \param filter_state FilterState instance to update
     \param start_value Start value
     \param end_value End value
     */
    void update_state(FilterState& filter_state, Variant const& start_value, Variant const& end_value) const;

    /**
     \brief Store start and end value in the filter state, without having an instance of RangeInputFilter.

     Updates filter_state with start and end values without an instance of RangeInputFilter. This is meant
     for the explanation of this method.
     to be used when creating a CannedQuery that points to a foreign scope.

     See the documentation of unity::scopes::update_state(FilterState const& filter_state, Variant const& start_value, Variant const& end_value)

     \param filter_state FilterState instance to update
     \param filter_id Unique identifier of filter
     \param start_value Start value
     \param end_value End value
     */
    static void update_state(FilterState& filter_state, std::string const& filter_id, Variant const& start_value, Variant const& end_value);

private:
    RangeInputFilter(internal::RangeInputFilterImpl*);
    internal::RangeInputFilterImpl* fwd() const;
    friend class internal::RangeInputFilterImpl;
};

} // namespace scopes

} // namespace unity

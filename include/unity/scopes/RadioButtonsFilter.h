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
#include <unity/scopes/FilterOption.h>

namespace unity
{

namespace scopes
{

class FilterState;

namespace internal
{

class RadioButtonsFilterImpl;

}

namespace experimental
{

/**
\brief A filter that displays mutually exclusive list of options.

Displays filter with a set of options and allows only one option to be selected at a time.
*/
class UNITY_API RadioButtonsFilter : public FilterBase
{
public:
/// @cond
    UNITY_DEFINES_PTRS(RadioButtonsFilter);
/// @endcond

    /**
    \brief Creates RadioButtonsFilter filter.

    Creates an empty RadioButtonsFilter filter. Use unity::scopes::RadioButtonsFilter::add_option() to add options to it.
    \param id A unique identifier for the filter that can be used to later identify it among several filters.
    \param label A display label for this filter
    \return Instance of RadioButtonsFilter
    */
    static RadioButtonsFilter::UPtr create(std::string const& id, std::string const& label);

    /**
     \brief Adds a new option to the filter.

     \param id A unique identifier of the option.
     \param label A display label for the option
     \return Instance of FilterOption
     */
    FilterOption::SCPtr add_option(std::string const& id, std::string const& label);

    /**
    \brief Get the label of this filter.
    \return The filter label.
    */
    std::string label() const;

    /**
    \brief Get active option from an instance of FilterState for this filter.
    \return The active option or nullptr if no option is active.
    */
    FilterOption::SCPtr active_option(FilterState const& filter_state) const;

    /**
    \brief Check if active options.
    \param filter_state The state of filters
    \return true if there is at least one option active
    */
    bool has_active_option(FilterState const& filter_state) const;


    /**
    \brief Get all options of this filter, in the order they were added.
    \return The list of options.
     */
    std::list<FilterOption::SCPtr> options() const;

    /**
    \brief Marks given FilterOption of this filter instance as active (or not active) in a FilterState object.

    Records the given FilterOption as "selected" in the FilterState. This is meant to be used to modify a
    FilterState received with a search request before sending it back to the client (UI shell).
    Only one option can be active at a time - marking an option active automatically deactivates any other option.
    */
    void update_state(FilterState& filter_state, FilterOption::SCPtr option, bool active) const;

    /**
    \brief Marks an option of a filter active/inactive in a FilterState object, without having an instance of OptionSelectorFilter.

    Updates an instance of FilterState, without the need for an OptionSelectorFilter instance. This is meant
    to be used when creating a canned Query that references another scope.
    */
    static void update_state(FilterState& filter_state, std::string const& filter_id, std::string const& option_id, bool value);

private:
    RadioButtonsFilter(internal::RadioButtonsFilterImpl*);
    internal::RadioButtonsFilterImpl* fwd() const;
    friend class internal::RadioButtonsFilterImpl;
};

} // namespace experimental

} // namespace scopes

} // namespace unity

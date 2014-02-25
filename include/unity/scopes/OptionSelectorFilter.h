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

#ifndef UNITY_SCOPES_OPTIONSELECTOR_H
#define UNITY_SCOPES_OPTIONSELECTOR_H

#include <unity/scopes/FilterBase.h>
#include <unity/scopes/FilterOption.h>
#include <unity/scopes/Variant.h>
#include <string>
#include <list>
#include <set>

namespace unity
{

namespace scopes
{
class FilterState;

namespace internal
{
class OptionSelectorFilterImpl;
class FilterBaseImpl;
}

/**
\brief Provides a list of choices, allows one or more to be selected.
A selection filter that displays a list of choices and allows one or more of them to be selected.
*/
class OptionSelectorFilter : public FilterBase
{
public:
/// @cond
    UNITY_DEFINES_PTRS(OptionSelectorFilter);
/// @endcond

    static OptionSelectorFilter::SPtr create(std::string const& id, std::string const& label, bool multi_select = false);

    /**
     \brief Get label of this filter.
     \return filter label
    */
    std::string label() const;

    /**
     \brief Check if this filter supports multiple options to be selected.
     \return true if multi-selection is enabled.
     */
    bool multi_select() const;

    /**
     \brief Add a new option to this filter.
     \return an option instance
     */
    FilterOption::SCPtr add_option(std::string const& id, std::string const& label);

    /**
     \brief Get all options of this filter, in the order they were added.
     \return list of options
     */
    std::list<FilterOption::SCPtr> options() const;

    /**
    \brief Get active options from an instance of FilterState for this filter.
    \return a set of selected filter options
    */
    std::set<FilterOption::SCPtr> active_options(FilterState const& filter_state) const;

    /**
     \brief Marks given FilterOption of this filter instance as active (or not active) in a FilterState object.
     Records given FilterOption as "selected" in the FilterState. This is meant to be used to modify FilterState
     received with search request before sending it back to the client (UI shell).
     */
    void update_state(FilterState& filter_state, FilterOption::SCPtr option, bool active) const;

    /**
    \brief Marks an option of a filter active/inactive in a FilterState object, without having an instance of OptionSelectorFilter.
    Updates an instance of FilterState, with no need of having an OptionSelectorFilter instance. This is meant
    to be used when creating a canned Query that references another scope.
    */
    static void update_state(FilterState& filter_state, std::string const& filter_id, std::string const& option_id, bool value);

private:
    OptionSelectorFilter(std::string const& id, std::string const& label, bool multi_select = false);
    OptionSelectorFilter(VariantMap const& var);
    internal::OptionSelectorFilterImpl* fwd() const;
    friend class internal::FilterBaseImpl;
};

} // namespace scopes

} // namespace unity

#endif

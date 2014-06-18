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

#ifndef UNITY_SCOPES_RATINGFILTER_H
#define UNITY_SCOPES_RATINGFILTER_H

#include <unity/scopes/FilterBase.h>
#include <unity/scopes/FilterOption.h>

namespace unity
{

namespace scopes
{

class FilterState;

namespace internal
{

class RatingFilterImpl;

}

namespace experimental
{

/**
\brief A filter that allows for rating-based selection.

Displays filter with a set of options, where every option has a label and icon.
Only one option can be active at a time. The active option uses the icon in "on" state
(see unity::scopes::RatingFilter::set_on_icon() ).  All other icons are "off" (see unity::scopes::RatingFilter::set_off_icon() ).
By default, "on" and "off" icons are bright and dim star respectively.

This filter is best suited for rating-based filtering.
*/
class UNITY_API RatingFilter : public FilterBase
{
public:
/// @cond
    UNITY_DEFINES_PTRS(RatingFilter);
/// @endcond

    /**
    \brief Creates RatingFilter widget.

    Creates an empty RatingFilter widget. Use unity::scopes::RatingFilter::add_option() to add rating options to it.
    \param id A unique identifier for the filter that can be used to identify it later among several filters.
    \param label A display label for this filter
    \return Instance of RatingFilter
    */
    static RatingFilter::UPtr create(std::string const& id, std::string const& label);

    /**
    \brief Creates RatingFilter widget.

    Creates a RatingFilter widget and populates it with some standard rating options.
    This is a convienience factory method, that fills RatingFilter in with options for ratings
    from 1 up to top_rating. Options are created with identifiers "1", "2" and so on, and labels
    "1+", "2+" etc., except for that last label, which is just the number (no plus sign). The maximum top_rating
    allowed is 10.

    \param id A unique identifier for the filter that can be used to identify it later among several filters.
    \param label A display label for this filter
    \param top_rating The maximum rating allowed.
    \return Instance of RatingFilter
    */
    static RatingFilter::UPtr create(std::string const& id, std::string const& label, int top_rating);

    /**
     \brief Adds a new option to the filter.

     \param id A unique identifier of the option.
     \param label A display label for the option
     \return Instance of FilterOption
     */
    FilterOption::SCPtr add_option(std::string const& id, std::string const& label);

    /**
     \brief Set a custom icon for the "on" state.

     If not set, a star icon will be used by default.

     \param on_icon icon uri
    */
    void set_on_icon(std::string const& on_icon);

    /**
     \brief Set a custom icon for the "off" state.

     If not set, a dim star icon will be used by default.

     \param off_icon icon uri
    */
    void set_off_icon(std::string const& off_icon);

    /**
    \brief Get the label of this filter.
    \return The filter label.
    */
    std::string label() const;

    /**
     \brief Get custom icon for the "on" state.

     If empty, then the default icon will be used.

     \return icon uri or empty string
     */
    std::string on_icon() const;

    /**
     \brief Get custom icon for the "off" state.

     If empty, then the default icon will be used.

     \return icon uri or empty string
     */
    std::string off_icon() const;

    /**
    \brief Get all options of this filter, in the order they were added.
    \return The list of options.
     */
    std::list<FilterOption::SCPtr> options() const;

    /**
    \brief Check if a rating option is active.
    \param filter_state The state of filters
    \return true if a rating option is active.
    */
    bool has_active_rating(FilterState const& filter_state) const;

    /**
    \brief Get active option from an instance of FilterState for this filter.
    \return The active option or nullptr if no option is active.
    */
    FilterOption::SCPtr active_rating(FilterState const& filter_state) const;

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
    RatingFilter(internal::RatingFilterImpl*);
    internal::RatingFilterImpl* fwd() const;
    friend class internal::RatingFilterImpl;
};

} // namespace experimental

} // namespace scopes

} // namespace unity

#endif

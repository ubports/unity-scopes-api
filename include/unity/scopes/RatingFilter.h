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

/**
\brief
*/
class UNITY_API RatingFilter : public FilterBase
{
public:
/// @cond
    UNITY_DEFINES_PTRS(RatingFilter);
/// @endcond

    /**
    \brief Creates RatingFilter widget.

    Creates a RatingFilter widget that allows values from 1 up to specified max value.
    Selected value is displayed using on_icon. Unselected values use off_icon.
    Values have numbers displayed next to them.

    \param id A unique identifier for the filter that can be used to identify it later among several filters.
    \param label A display label for this filter,
    \param max The maximum rating allowed.
    \param on_icon An icon to use for selected rating (star symbol by default).
    \param off_icon An icon to use for unselected rating (dimmed star symbol by default).
    \return Instance of RatingFilter
    */
    static RatingFilter::SPtr create(std::string const& id, std::string const& label, int top_rating = 5);

    /**
    \brief Creates RatingFilter widget.

    Creates a RatingFilter widget that allows values from 1 up to the value inferred from the number of elements of value_labels.
    Selected value is displayed using on_icon. Unselected values use off_icon.
    Values have lables displayed next to them, as specified by value_labels.

    \param id A unique identifier for the filter that can be used to identify it later among several filters.
    \param label A display label for this filter,
    \param value_labels Labels to be displayed along rating values.
    \param on_icon An icon to use for selected rating (star symbol by default).
    \param off_icon An icon to use for unselected rating (dimmed star symbol by default).
    \return Instance of RatingFilter
    */

    static RatingFilter::SPtr create(std::string const& id, std::string const& label);

    FilterOption::SCPtr add_option(std::string const& id, std::string const& label);
    void set_on_icon(std::string const& on_icon);
    void set_off_icon(std::string const& off_icon);

    std::string on_icon() const;
    std::string off_icon() const;

    FilterOption::SCPtr active_rating(FilterState const& filter_state) const;

    void update_state(FilterState& filter_state, FilterOption::SCPtr option, bool active) const;
    static void update_state(FilterState& filter_state, std::string const& filter_id, std::string const& option_id, bool value);

private:
    RatingFilter(internal::RatingFilterImpl*);
    internal::RatingFilterImpl* fwd() const;
    friend class internal::RatingFilterImpl;
};

} // namespace scopes

} // namespace unity

#endif

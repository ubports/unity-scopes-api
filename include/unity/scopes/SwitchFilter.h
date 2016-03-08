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

namespace unity
{

namespace scopes
{

class FilterState;

namespace internal
{
class SwitchFilterImpl;
}

namespace experimental
{

/**
\brief A simple on/off switch filter.

This is a simple on/off switcher. The filter is initially off.
*/
class UNITY_API SwitchFilter : public FilterBase
{
public:
/// @cond
    UNITY_DEFINES_PTRS(SwitchFilter);
/// @endcond

    /**
    \brief Creates a SwitchFilter
    \param id A unique identifier for the filter that can be used to later identify it among several filters.
    \param label A display label for the filter.
    \return SwitchFilter instance
    */
    static SwitchFilter::UPtr create(std::string const& id, std::string const& label);

    /**
    \brief Get the label of this filter.
    \return The filter label.
    */
    std::string label() const;

    /**
    \brief Check if the filter is "on".

    The filter is "off" by default and also if not present in the filter_state.

    \param filter_state The state of filters
    \return true if the fiter is on.
    */
    bool is_on(FilterState const& filter_state) const;

    /**
    \brief Sets on/off state of this filter instance in a FilterState object.

    \param filter_state The state of filters
    \param active the value of the filter (on/off)
    */
    void update_state(FilterState& filter_state, bool active) const;

    /**
    \brief Sets on/off state of a SwitchFilter in a FilterState object,  without having an instance of SwitchFilter.

    Updates an instance of FilterState, without the need for an SwitchFilter instance. This is meant
    to be used when creating a canned Query that references another scope.
    */
    static void update_state(FilterState& filter_state, std::string const& filter_id, bool active);

private:
    SwitchFilter(internal::SwitchFilterImpl*);
    internal::SwitchFilterImpl* fwd() const;
    friend class internal::SwitchFilterImpl;
};

} // namespace experimental

} // namespace scopes

} // namespace unity

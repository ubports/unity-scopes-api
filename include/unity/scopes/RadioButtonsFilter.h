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

#ifndef UNITY_SCOPES_RADIOBUTTONSFILTER_H
#define UNITY_SCOPES_RADIOBUTTONSFILTER_H

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

/**
\brief
*/
class UNITY_API RadioButtonsFilter : public FilterBase
{
public:
/// @cond
    UNITY_DEFINES_PTRS(RadioButtonsFilter);
/// @endcond

    static RadioButtonsFilter::SPtr create(std::string const& id, std::string const& label);

    FilterOption::SCPtr add_option(std::string const& id, std::string const& label);

    std::string label() const;
    FilterOption::SCPtr active_option(FilterState const& filter_state) const;
    std::list<FilterOption::SCPtr> options() const;

    void update_state(FilterState& filter_state, FilterOption::SCPtr option, bool active) const;
    static void update_state(FilterState& filter_state, std::string const& filter_id, std::string const& option_id, bool value);

private:
    RadioButtonsFilter(internal::RadioButtonsFilterImpl*);
    internal::RadioButtonsFilterImpl* fwd() const;
    friend class internal::RadioButtonsFilterImpl;
};

} // namespace scopes

} // namespace unity

#endif

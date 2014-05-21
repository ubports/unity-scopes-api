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

#ifndef UNITY_SCOPES_SWITCHFILTER_H
#define UNITY_SCOPES_SWITCHFILTER_H

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

/**
\brief
*/
class UNITY_API SwitchFilter : public FilterBase
{
public:
/// @cond
    UNITY_DEFINES_PTRS(SwitchFilter);
/// @endcond

    static SwitchFilter::UPtr create(std::string const& id, std::string const& label);

    /**
    \brief Get the label of this filter.
    \return The filter label.
    */
    std::string label() const;

    bool is_on(FilterState const& filter_state) const;
    void update_state(FilterState& filter_state, bool active) const;
    static void update_state(FilterState& filter_state, std::string const& filter_id, bool active);

private:
    SwitchFilter(internal::SwitchFilterImpl*);
    internal::SwitchFilterImpl* fwd() const;
    friend class internal::SwitchFilterImpl;
};

} // namespace scopes

} // namespace unity

#endif

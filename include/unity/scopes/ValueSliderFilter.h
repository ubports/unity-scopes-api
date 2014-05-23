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

/**
\brief
*/
class UNITY_API ValueSliderFilter : public FilterBase
{
public:
    /// @cond
    UNITY_DEFINES_PTRS(ValueSliderFilter);
    /// @endcond

    enum SliderType
    {
        LessThan,
        MoreThan
    };

    static ValueSliderFilter::UPtr create(std::string const& id, std::string const& label_template, int min, int max);

    void set_slider_type(SliderType tp);
    SliderType slider_type() const;
    int min() const;
    int max() const;
    std::string label(FilterState const& filter_state) const;
    std::string label_template() const;
    bool has_value(FilterState const& filter_state) const;
    int value(FilterState const& filter_state) const;
    void update_state(FilterState& filter_state, int value) const;
    static void update_state(FilterState& filter_state, std::string const& filter_id, int value);

private:
    ValueSliderFilter(internal::ValueSliderFilterImpl*);
    internal::ValueSliderFilterImpl* fwd() const;
    friend class internal::ValueSliderFilterImpl;
};

} // namespace scopes

} // namespace unity

#endif

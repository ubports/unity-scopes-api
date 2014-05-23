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

#include <unity/scopes/ValueSliderFilter.h>
#include <unity/scopes/internal/ValueSliderFilterImpl.h>

namespace unity
{

namespace scopes
{

ValueSliderFilter::ValueSliderFilter(internal::ValueSliderFilterImpl *impl)
    : FilterBase(impl)
{
}

ValueSliderFilter::UPtr ValueSliderFilter::create(std::string const& id, std::string const& label_template, int min, int max)
{
    return std::unique_ptr<ValueSliderFilter>(new ValueSliderFilter(new internal::ValueSliderFilterImpl(id, label_template, min, max)));
}

void ValueSliderFilter::set_slider_type(SliderType tp)
{
    fwd()->set_slider_type(tp);
}

ValueSliderFilter::SliderType ValueSliderFilter::slider_type() const
{
    return fwd()->slider_type();
}

int ValueSliderFilter::min() const
{
    return fwd()->min();
}

int ValueSliderFilter::max() const
{
    return fwd()->max();
}

std::string ValueSliderFilter::label(FilterState const& filter_state) const
{
    return fwd()->label(filter_state);
}

std::string ValueSliderFilter::label_template() const
{
    return fwd()->label_template();
}

bool ValueSliderFilter::has_value(FilterState const& filter_state) const
{
    return fwd()->has_value(filter_state);
}

int ValueSliderFilter::value(FilterState const& filter_state) const
{
    return fwd()->value(filter_state);
}

void ValueSliderFilter::update_state(FilterState& filter_state, int value) const
{
    fwd()->update_state(filter_state, value);
}

void ValueSliderFilter::update_state(FilterState& filter_state, std::string const& filter_id, int value)
{
    internal::ValueSliderFilterImpl::update_state(filter_state, filter_id, value);
}

internal::ValueSliderFilterImpl* ValueSliderFilter::fwd() const
{
    return dynamic_cast<internal::ValueSliderFilterImpl*>(p.get());
}

} // namespace scopes

} // namespace unity

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

#include <unity/scopes/internal/ValueSliderFilterImpl.h>
#include <unity/scopes/FilterState.h>
#include <unity/UnityExceptions.h>
#include <boost/format.hpp>
#include <sstream>

namespace unity
{

namespace scopes
{

namespace internal
{

ValueSliderFilterImpl::ValueSliderFilterImpl(std::string const& id, std::string const& label_template, int min, int max)
    : FilterBaseImpl(id),
      min_(min),
      max_(max)
{
    if (min < 0 || min >= max)
    {
        std::stringstream err;
        err << "ValueSliderFilterImpl::ValueSliderFilterImpl(): invalid min or max value for filter '" << id << "', min is " << min << ", max is " << max;
        throw unity::LogicException(err.str());
    }
    default_val_ = (min + max) / 2;
}

void ValueSliderFilterImpl::set_default_value(int val)
{
    check_range(val);
    default_val_ = val;
}

void ValueSliderFilterImpl::set_slider_type(ValueSliderFilter::SliderType tp)
{
    slider_type_ = tp;
}

ValueSliderFilter::SliderType ValueSliderFilterImpl::slider_type() const
{
    return slider_type_;
}

std::string ValueSliderFilterImpl::label(FilterState const& filter_state) const
{
    int val;
    try
    {
        val = get_value(filter_state);
    }
    catch (unity::LogicException const&)
    {
        val = default_val_;
    }
    return boost::str(boost::format(label_template_) % val);
}

std::string ValueSliderFilterImpl::label_template() const
{
    return label_template_;
}

bool ValueSliderFilterImpl::has_value(FilterState const& filter_state) const
{
    if (filter_state.has_filter(id()))
    {
        return FilterBaseImpl::get(filter_state, id()).which() == Variant::Type::Int;
    }
    return false;
}

int ValueSliderFilterImpl::get_value(FilterState const& filter_state) const
{
    if (filter_state.has_filter(id()))
    {
        try
        {
            int val = FilterBaseImpl::get(filter_state, id()).get_int(); // this can throw if of different type
            check_range(val);
            return val;
        }
        catch (...)
        {
            // ignore all errors here - we could be getting an incorrect value for this filter
            // via a canned query from another scope, we shouldn't break this scope on it.
        }
    }
    std::stringstream err;
    err << "ValueSliderFilterImpl::get_value(): value is not set for filter '" << id() << "'";
    throw unity::LogicException(err.str());
}

void ValueSliderFilterImpl::update_state(FilterState& filter_state, int value) const
{
}

void ValueSliderFilterImpl::update_state(FilterState& filter_state, std::string const& filter_id, int value)
{
}

void ValueSliderFilterImpl::serialize(VariantMap& var) const
{
}

void ValueSliderFilterImpl::deserialize(VariantMap const& var)
{
}

std::string ValueSliderFilterImpl::filter_type() const
{
    return "value_slider";
}

void ValueSliderFilterImpl::check_range(int val) const
{
    if (val < min_ || val > max_)
    {
        std::stringstream err;
        err << "ValueSliderFilterImpl::check_range(): value " << val << " outside of allowed range (" << min_ << ", " << max_ << ")";
        throw unity::LogicException(err.str());
    }
}

} // namespace internal

} // namespace scopes

} // namespace unity

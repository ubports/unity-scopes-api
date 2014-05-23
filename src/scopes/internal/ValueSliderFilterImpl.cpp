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
#include <unity/scopes/internal/Utils.h>
#include <unity/UnityExceptions.h>
#include <boost/format.hpp>
#include <sstream>

namespace unity
{

namespace scopes
{

namespace internal
{

ValueSliderFilterImpl::ValueSliderFilterImpl(std::string const& id, std::string const& label, std::string const& label_template, double min, double max)
    : FilterBaseImpl(id),
      label_(label),
      label_template_(label_template),
      slider_type_(ValueSliderFilter::SliderType::LessThan),
      min_(min),
      max_(max)
{
    if (label.empty())
    {
        throw InvalidArgumentException("ValueSliderFilterImpl(): Invalid empty label string");
    }

    if (min < 0 || min >= max)
    {
        std::stringstream err;
        err << "ValueSliderFilterImpl::ValueSliderFilterImpl(): invalid min or max value for filter '" << id << "', min is " << min << ", max is " << max;
        throw LogicException(err.str());
    }
    default_val_ = max;
}

ValueSliderFilterImpl::ValueSliderFilterImpl(VariantMap const& var)
    : FilterBaseImpl(var)
{
    deserialize(var);
}

ValueSliderFilter::SPtr ValueSliderFilterImpl::create(VariantMap const& var)
{
    return std::shared_ptr<ValueSliderFilter>(new ValueSliderFilter(new ValueSliderFilterImpl(var)));
}

void ValueSliderFilterImpl::set_default_value(double val)
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

double ValueSliderFilterImpl::default_value() const
{
    return default_val_;
}

double ValueSliderFilterImpl::min() const
{
    return min_;
}

double ValueSliderFilterImpl::max() const
{
    return max_;
}

std::string ValueSliderFilterImpl::label() const
{
    return label_;
}

std::string ValueSliderFilterImpl::value_label(FilterState const& filter_state) const
{
    double val;
    try
    {
        val = value(filter_state);
    }
    catch (LogicException const&)
    {
        val = default_val_;
    }

    try
    {
        return boost::str(boost::format(label_template_) % val);
    }
    catch (boost::io::format_error const& e)
    {
        std::stringstream err;
        err << "ValueSliderFilterImpl::label(): Failed to format label of filter '" << id() << "' using template '" << label_template_ << "'";
        throw LogicException(err.str());
    }
}

std::string ValueSliderFilterImpl::value_label_template() const
{
    return label_template_;
}

bool ValueSliderFilterImpl::has_value(FilterState const& filter_state) const
{
    return filter_state.has_filter(id()) && FilterBaseImpl::get(filter_state, id()).which() == Variant::Type::Double;
}

double ValueSliderFilterImpl::value(FilterState const& filter_state) const
{
    if (filter_state.has_filter(id()))
    {
        try
        {
            double val = FilterBaseImpl::get(filter_state, id()).get_double(); // this can throw if of different type
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
    throw LogicException(err.str());
}

void ValueSliderFilterImpl::update_state(FilterState& filter_state, double value) const
{
    check_range(value);
    update_state(filter_state, id(), value);
}

void ValueSliderFilterImpl::update_state(FilterState& filter_state, std::string const& filter_id, double value)
{
    VariantMap& state = FilterBaseImpl::get(filter_state);
    state[filter_id] = Variant(value);
}

void ValueSliderFilterImpl::serialize(VariantMap& var) const
{
    var["label"] = label_;
    var["label_template"] = label_template_;
    var["min"] = Variant(min_);
    var["max"] = Variant(max_);
    var["default"] = Variant(default_val_);
    var["slider_type"] = static_cast<int>(slider_type_);
}

void ValueSliderFilterImpl::deserialize(VariantMap const& var)
{
    auto it = find_or_throw("ValueSliderFilterImpl::deserialize()", var, "label");
    label_ = it->second.get_string();
    it = find_or_throw("ValueSliderFilterImpl::deserialize()", var, "label_template");
    label_template_ = it->second.get_string();
    it = find_or_throw("ValueSliderFilterImpl::deserialize()", var, "min");
    min_ = it->second.get_double();
    it = find_or_throw("ValueSliderFilterImpl::deserialize()", var, "max");
    max_ = it->second.get_double();
    it = find_or_throw("ValueSliderFilterImpl::deserialize()", var, "default");
    default_val_ = it->second.get_double();
    it = find_or_throw("ValueSliderFilterImpl::deserialize()", var, "slider_type");
    slider_type_ = static_cast<ValueSliderFilter::SliderType>(it->second.get_int());
}

std::string ValueSliderFilterImpl::filter_type() const
{
    return "value_slider";
}

void ValueSliderFilterImpl::check_range(double val) const
{
    if (val < min_ || val > max_)
    {
        std::stringstream err;
        err << "ValueSliderFilterImpl::check_range(): value " << val << " outside of allowed range (" << min_ << ", " << max_ << ")";
        throw LogicException(err.str());
    }
}

} // namespace internal

} // namespace scopes

} // namespace unity

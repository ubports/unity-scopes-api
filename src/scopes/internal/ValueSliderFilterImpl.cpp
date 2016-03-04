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
#include <unity/scopes/ScopeExceptions.h>
#include <unity/UnityExceptions.h>
#include <sstream>

namespace unity
{

namespace scopes
{

namespace internal
{

ValueSliderFilterImpl::ValueSliderFilterImpl(std::string const& id, double min, double max, double default_value, ValueSliderLabels const& labels)
    : FilterBaseImpl(id),
      min_(min),
      max_(max),
      default_val_(default_value),
      labels_(new ValueSliderLabels(labels))
{
    if (default_value < min || default_value > max)
    {
        std::stringstream err;
        err << "ValueSliderFilterImpl::ValueSliderFilterImpl(): invalid default value for filter '" << id << "', " << default_value << " not in " << min << "-"
            << max << " range";
        throw InvalidArgumentException(err.str());
    }
    labels_->p->validate(min_, max_);
}

ValueSliderFilterImpl::ValueSliderFilterImpl(VariantMap const& var)
    : FilterBaseImpl(var)
{
    deserialize(var);
}

ValueSliderFilter::SPtr ValueSliderFilterImpl::create(VariantMap const& var)
{
    auto filter = std::shared_ptr<ValueSliderFilter>(new ValueSliderFilter(new ValueSliderFilterImpl(var)));
    filter->fwd()->validate_display_hints();
    return filter;
}

void ValueSliderFilterImpl::set_default_value(double val)
{
    check_range(val);
    default_val_ = val;
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

ValueSliderLabels const& ValueSliderFilterImpl::labels() const
{
    // labels_ pointer is guaranteed to be initialized either by the regular ctor or when deserialized from a variant
    assert(labels_);
    return *labels_;
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
            auto val = FilterBaseImpl::get(filter_state, id()).get_double(); // this can throw if of different type
            check_range(val);
            return val;
        }
        catch (...)
        {
            // ignore all errors here - we could be getting an incorrect value for this filter
            // via a canned query from another scope, we shouldn't break this scope on it.
        }
    }
    throw NotFoundException("ValueSliderFilterImpl::get_value(): value is not set for filter", id());
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

void ValueSliderFilterImpl::validate_display_hints() const
{
    if (display_hints() & FilterBase::DisplayHints::Primary)
    {
        throw unity::InvalidArgumentException("ValueSliderFilter::set_display_hints(): primary navigation flag is not supported by this filter type");
    }
}

void ValueSliderFilterImpl::serialize(VariantMap& var) const
{
    var["min"] = Variant(min_);
    var["max"] = Variant(max_);
    var["default"] = Variant(default_val_);
    var["labels"] = Variant(labels_->serialize());
}

void ValueSliderFilterImpl::deserialize(VariantMap const& var)
{
    auto it = find_or_throw("ValueSliderFilterImpl::deserialize()", var, "min");
    min_ = it->second.get_double();
    it = find_or_throw("ValueSliderFilterImpl::deserialize()", var, "max");
    max_ = it->second.get_double();
    it = find_or_throw("ValueSliderFilterImpl::deserialize()", var, "default");
    default_val_ = it->second.get_double();
    it = find_or_throw("ValueSliderFilterImpl::deserialize()", var, "labels");
    labels_.reset(new ValueSliderLabels(it->second.get_dict()));
    labels_->p->validate(min_, max_);
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
        throw InvalidArgumentException(err.str());
    }
}

} // namespace internal

} // namespace scopes

} // namespace unity

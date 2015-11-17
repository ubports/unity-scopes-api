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

#include <unity/scopes/internal/RangeInputFilterImpl.h>
#include <unity/scopes/FilterState.h>
#include <unity/scopes/internal/Utils.h>
#include <unity/scopes/ScopeExceptions.h>
#include <unity/UnityExceptions.h>

namespace unity
{

namespace scopes
{

namespace internal
{

RangeInputFilterImpl::RangeInputFilterImpl(std::string const& id,
        Variant const& default_start_value, Variant const& default_end_value,
        std::string const& start_prefix_label, std::string const& start_postfix_label,
        std::string const& central_label,
        std::string const& end_prefix_label, std::string const& end_postfix_label)
    : FilterBaseImpl(id),
      default_start_value_(default_start_value),
      default_end_value_(default_end_value),
      start_prefix_label_(start_prefix_label),
      start_postfix_label_(start_postfix_label),
      end_prefix_label_(end_prefix_label),
      end_postfix_label_(end_postfix_label),
      central_label_(central_label)
{
}

RangeInputFilterImpl::RangeInputFilterImpl(VariantMap const& var)
    : FilterBaseImpl(var)
{
    deserialize(var);
}

RangeInputFilter::SPtr RangeInputFilterImpl::create(VariantMap const& var)
{
    auto filter = std::shared_ptr<RangeInputFilter>(new RangeInputFilter(new RangeInputFilterImpl(var)));
    filter->fwd()->validate_display_hints();
    return filter;
}

std::string RangeInputFilterImpl::start_prefix_label() const
{
    return start_prefix_label_;
}

std::string RangeInputFilterImpl::start_postfix_label() const
{
    return start_postfix_label_;
}

std::string RangeInputFilterImpl::end_prefix_label() const
{
    return end_prefix_label_;
}

std::string RangeInputFilterImpl::end_postfix_label() const
{
    return end_postfix_label_;
}

std::string RangeInputFilterImpl::central_label() const
{
    return central_label_;
}

Variant RangeInputFilterImpl::default_start_value() const
{
    return default_start_value_;
}

Variant RangeInputFilterImpl::default_end_value() const
{
    return default_end_value_;
}

bool RangeInputFilterImpl::has_value(FilterState const& filter_state, unsigned int index) const
{
    try
    {
        auto var = FilterBaseImpl::get(filter_state, id()).get_array();
        return var.size() > index && (var[index].which() == Variant::Type::Int
                || var[index].which() == Variant::Type::Double);
    }
    catch (...)
    {
    }
    return false;
}

bool RangeInputFilterImpl::has_start_value(FilterState const& filter_state) const
{
    return has_value(filter_state, 0);
}

bool RangeInputFilterImpl::has_end_value(FilterState const& filter_state) const
{
    return has_value(filter_state, 1);
}

double RangeInputFilterImpl::get_value(FilterState const& filter_state, unsigned int index) const
{
    if (filter_state.has_filter(id()))
    {
        try
        {
            auto var = FilterBaseImpl::get(filter_state, id()).get_array();
            if (var.size() > index)
            {
                if (var[index].which() == Variant::Type::Double)
                {
                    return var[index].get_double();
                }
                if (var[index].which() == Variant::Type::Int)
                {
                    return var[index].get_int();
                }
            }
        }
        catch (...)
        {
        }
    }
    throw unity::scopes::NotFoundException("RangeInputFilterImpl::get_value(): invalid index for filter '" + id() + "'",
                                           std::to_string(index));
}

double RangeInputFilterImpl::start_value(FilterState const& filter_state) const
{
    return get_value(filter_state, 0);
}

double RangeInputFilterImpl::end_value(FilterState const& filter_state) const
{
    return get_value(filter_state, 1);
}

void RangeInputFilterImpl::check_type(Variant const& val, std::string const& filter_id, std::string const& varname)
{
    if (val.which() == Variant::Type::Int || val.which() == Variant::Type::Double || val.is_null())
        return;
    std::stringstream err;
    err << "RangeInputFilterImpl::check_type(): Invalid variant type for " << varname << ", filter '" << filter_id << "'";
    throw unity::InvalidArgumentException(err.str());
}

void RangeInputFilterImpl::update_state(FilterState& filter_state, Variant const& start_value, Variant const& end_value) const
{
    update_state(filter_state, id(), start_value, end_value);
}

void RangeInputFilterImpl::update_state(FilterState& filter_state, std::string const& filter_id, Variant const& start_value, Variant const& end_value)
{
    check_type(start_value, filter_id, "start_value");
    check_type(end_value, filter_id, "end_value");

    if (start_value.is_null() && end_value.is_null())
    {
        filter_state.remove(filter_id);
    }
    else
    {
        if (!(start_value.is_null() || end_value.is_null()))
        {
            const double start = start_value.which() == Variant::Type::Double ? start_value.get_double() : start_value.get_int();
            const double end = end_value.which() == Variant::Type::Double ? end_value.get_double() : end_value.get_int();
            if (start >= end)
            {
                std::stringstream err;
                err << "RangeInputFilterImpl::update_state(): start_value " << start << " is greater or equal to end_value " << end << " for filter " <<
                    filter_id;
                throw unity::LogicException(err.str());
            }
        }
        VariantMap& state = FilterBaseImpl::get(filter_state);
        const VariantArray arr({start_value, end_value});
        state[filter_id] = arr;
    }
}

std::string RangeInputFilterImpl::filter_type() const
{
    return "range_input";
}

void RangeInputFilterImpl::serialize(VariantMap& var) const
{
    var["default_start_value"] = default_start_value_;
    var["default_end_value"] = default_end_value_;
    var["start_prefix_label"] = start_prefix_label_;
    var["start_postfix_label"] = start_postfix_label_;
    var["end_prefix_label"] = end_prefix_label_;
    var["end_postfix_label"] = end_postfix_label_;
    var["central_label"] = central_label_;
}

void RangeInputFilterImpl::deserialize(VariantMap const& var)
{
    auto it = find_or_throw("RangeInputFilterImpl::deserialize()", var, "start_prefix_label");
    start_prefix_label_ = it->second.get_string();
    it = find_or_throw("RangeInputFilterImpl::deserialize()", var, "start_postfix_label");
    start_postfix_label_ = it->second.get_string();
    it = find_or_throw("RangeInputFilterImpl::deserialize()", var, "end_prefix_label");
    end_prefix_label_ = it->second.get_string();
    it = find_or_throw("RangeInputFilterImpl::deserialize()", var, "end_postfix_label");
    end_postfix_label_ = it->second.get_string();
    it = find_or_throw("RangeInputFilterImpl::deserialize()", var, "central_label");
    central_label_ = it->second.get_string();
    it = find_or_throw("RangeInputFilterImpl::deserialize()", var, "default_start_value");
    default_start_value_ = it->second;
    it = find_or_throw("RangeInputFilterImpl::deserialize()", var, "default_end_value");
    default_end_value_ = it->second;
}

void RangeInputFilterImpl::validate_display_hints() const
{
    if (display_hints() & FilterBase::DisplayHints::Primary)
    {
        throw unity::InvalidArgumentException("RangeInputFilter::set_display_hints(): primary navigation flag is not supported by this filter type");
    }
}

} // namespace internal

} // namespace scopes

} // namespace unity

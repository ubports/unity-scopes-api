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
#include <unity/UnityExceptions.h>
#include <sstream>

namespace unity
{

namespace scopes
{

namespace internal
{

RangeInputFilterImpl::RangeInputFilterImpl(std::string const& id, std::string const& start_label, std::string const& end_label, std::string const& unit_label)
    : FilterBaseImpl(id),
      start_label_(start_label),
      end_label_(end_label),
      unit_label_(unit_label)
{
}

RangeInputFilterImpl::RangeInputFilterImpl(VariantMap const& var)
    : FilterBaseImpl(var)
{
    deserialize(var);
}

RangeInputFilter::SPtr RangeInputFilterImpl::create(VariantMap const& var)
{
    return std::shared_ptr<RangeInputFilter>(new RangeInputFilter(new RangeInputFilterImpl(var)));
}

std::string RangeInputFilterImpl::start_label() const
{
    return start_label_;
}

std::string RangeInputFilterImpl::end_label() const
{
    return end_label_;
}

std::string RangeInputFilterImpl::unit_label() const
{
    return unit_label_;
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
    std::stringstream err;
    err << "RangeInputFilterImpl::get_value(): value is not set for filter '" << id() << "', index " << index;
    throw unity::LogicException(err.str());
}

double RangeInputFilterImpl::start_value(FilterState const& filter_state) const
{
    return get_value(filter_state, 0);
}

double RangeInputFilterImpl::end_value(FilterState const& filter_state) const
{
    return get_value(filter_state, 1);
}

void RangeInputFilterImpl::update_state(FilterState& filter_state, Variant const& start_value, Variant const& end_value) const
{
    update_state(filter_state, id(), start_value, end_value);
}

void RangeInputFilterImpl::update_state(FilterState& filter_state, std::string const& filter_id, Variant const& start_value, Variant const& end_value)
{
    VariantMap& state = FilterBaseImpl::get(filter_state);
    const VariantArray arr({start_value, end_value});
    state[filter_id] = arr;
}

std::string RangeInputFilterImpl::filter_type() const
{
    return "range_input";
}

void RangeInputFilterImpl::serialize(VariantMap& var) const
{
    VariantArray ops = VariantArray();
    var["start_label"] = start_label_;
    var["end_label"] = end_label_;
    var["unit_label"] = unit_label_;
}

void RangeInputFilterImpl::deserialize(VariantMap const& var)
{
    auto it = find_or_throw("RangeInputFilterImpl::deserialize()", var, "start_label");
    start_label_ = it->second.get_string();
    it = find_or_throw("RangeInputFilterImpl::deserialize()", var, "end_label");
    end_label_ = it->second.get_string();
    it = find_or_throw("RangeInputFilterImpl::deserialize()", var, "unit_label");
    unit_label_ = it->second.get_string();
}

} // namespace internal

} // namespace scopes

} // namespace unity

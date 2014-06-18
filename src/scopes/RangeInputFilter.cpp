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

#include <unity/scopes/RangeInputFilter.h>
#include <unity/scopes/FilterState.h>
#include <unity/scopes/internal/RangeInputFilterImpl.h>

namespace unity
{

namespace scopes
{

namespace experimental
{

RangeInputFilter::SPtr RangeInputFilter::create(std::string const& id, std::string const& start_label, std::string const& end_label, std::string const& unit_label)
{
    return std::shared_ptr<RangeInputFilter>(new RangeInputFilter(new internal::RangeInputFilterImpl(id, start_label, end_label, unit_label)));
}

RangeInputFilter::RangeInputFilter(internal::RangeInputFilterImpl* impl)
    : FilterBase(impl)
{
}

std::string RangeInputFilter::start_label() const
{
    return fwd()->start_label();
}

std::string RangeInputFilter::end_label() const
{
    return fwd()->end_label();
}

std::string RangeInputFilter::unit_label() const
{
    return fwd()->unit_label();
}

bool RangeInputFilter::has_start_value(FilterState const& filter_state) const
{
    return fwd()->has_start_value(filter_state);
}

bool RangeInputFilter::has_end_value(FilterState const& filter_state) const
{
    return fwd()->has_end_value(filter_state);
}

double RangeInputFilter::start_value(FilterState const& filter_state) const
{
    return fwd()->start_value(filter_state);
}

double RangeInputFilter::end_value(FilterState const& filter_state) const
{
    return fwd()->end_value(filter_state);
}

void RangeInputFilter::update_state(FilterState& filter_state, Variant const& start_value, Variant const& end_value) const
{
    fwd()->update_state(filter_state, start_value, end_value);
}

void RangeInputFilter::update_state(FilterState& filter_state, std::string const& filter_id, Variant const& start_value, Variant const& end_value)
{
    internal::RangeInputFilterImpl::update_state(filter_state, filter_id, start_value, end_value);
}

internal::RangeInputFilterImpl* RangeInputFilter::fwd() const
{
    return dynamic_cast<internal::RangeInputFilterImpl*>(p.get());
}

} // namespace experimental

} // namespace scopes

} // namespace unity

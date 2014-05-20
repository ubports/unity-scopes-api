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

#include <unity/scopes/RadioButtonsFilter.h>
#include <unity/scopes/internal/RadioButtonsFilterImpl.h>

namespace unity
{

namespace scopes
{

RadioButtonsFilter::RadioButtonsFilter(internal::RadioButtonsFilterImpl *impl)
    : FilterBase(impl)
{
}

FilterOption::SCPtr RadioButtonsFilter::add_option(std::string const& id, std::string const& label)
{
    return fwd()->add_option(id, label);
}

std::string RadioButtonsFilter::label() const
{
    return fwd()->label();
}

FilterOption::SCPtr RadioButtonsFilter::active_option(FilterState const& filter_state) const
{
    return fwd()->active_option(filter_state);
}

bool RadioButtonsFilter::has_active_option(FilterState const& filter_state) const
{
    return fwd()->has_active_option(filter_state);
}

std::list<FilterOption::SCPtr> RadioButtonsFilter::options() const
{
    return fwd()->options();
}

void RadioButtonsFilter::update_state(FilterState& filter_state, FilterOption::SCPtr option, bool active) const
{
    fwd()->update_state(filter_state, option, active);
}

void RadioButtonsFilter::update_state(FilterState& filter_state, std::string const& filter_id, std::string const& option_id, bool value)
{
    internal::RadioButtonsFilterImpl::update_state(filter_state, filter_id, option_id, value);
}

RadioButtonsFilter::UPtr RadioButtonsFilter::create(std::string const& id, std::string const& label)
{
    return std::unique_ptr<RadioButtonsFilter>(new RadioButtonsFilter(new internal::RadioButtonsFilterImpl(id, label)));
}

internal::RadioButtonsFilterImpl* RadioButtonsFilter::fwd() const
{
    return dynamic_cast<internal::RadioButtonsFilterImpl*>(p.get());
}

} // namespace scopes

} // namespace unity

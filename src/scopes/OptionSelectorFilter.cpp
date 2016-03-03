/*
 * Copyright (C) 2013 Canonical Ltd
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

#include <unity/scopes/OptionSelectorFilter.h>
#include <unity/scopes/internal/OptionSelectorFilterImpl.h>

namespace unity
{

namespace scopes
{

OptionSelectorFilter::OptionSelectorFilter(internal::OptionSelectorFilterImpl *impl)
    : FilterBase(impl)
{
}

OptionSelectorFilter::UPtr OptionSelectorFilter::create(std::string const& id, std::string const& label, bool multi_select)
{
    return std::unique_ptr<OptionSelectorFilter>(new OptionSelectorFilter(new internal::OptionSelectorFilterImpl(id, label, multi_select)));
}

std::string OptionSelectorFilter::label() const
{
    return fwd()->label();
}

bool OptionSelectorFilter::multi_select() const
{
    return fwd()->multi_select();
}

FilterOption::SCPtr OptionSelectorFilter::add_option(std::string const& id, std::string const& label)
{
    return fwd()->add_option(id, label, false);
}

FilterOption::SCPtr OptionSelectorFilter::add_option(std::string const& id, std::string const& label, bool value)
{
    return fwd()->add_option(id, label, value);
}

std::list<FilterOption::SCPtr> OptionSelectorFilter::options() const
{
    return fwd()->options();
}

bool OptionSelectorFilter::has_active_option(FilterState const& filter_state) const
{
    return fwd()->has_active_option(filter_state);
}

std::set<FilterOption::SCPtr> OptionSelectorFilter::active_options(FilterState const& filter_state) const
{
    return fwd()->active_options(filter_state);
}

void OptionSelectorFilter::update_state(FilterState& filter_state, FilterOption::SCPtr option, bool active) const
{
    fwd()->update_state(filter_state, option, active);
}

void OptionSelectorFilter::update_state(FilterState& filter_state, std::string const& filter_id, std::string const& option_id, bool value)
{
    internal::OptionSelectorFilterImpl::update_state(filter_state, filter_id, option_id, value);
}

internal::OptionSelectorFilterImpl* OptionSelectorFilter::fwd() const
{
    return dynamic_cast<internal::OptionSelectorFilterImpl*>(p.get());
}

} // namespace scopes

} // namespace unity

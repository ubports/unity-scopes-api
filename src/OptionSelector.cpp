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

#include <scopes/OptionSelector.h>
#include <scopes/internal/OptionSelectorImpl.h>

namespace unity
{

namespace api
{

namespace scopes
{

OptionSelector::OptionSelector(std::string const& id, std::string const& label, bool multi_select)
    : FilterBase(new internal::OptionSelectorImpl(id, label, multi_select))
{
}

std::string OptionSelector::label() const
{
    return fwd()->label();
}

bool OptionSelector::multi_select() const
{
    return fwd()->multi_select();
}

FilterOption::SCPtr OptionSelector::add_option(std::string const& id, std::string const& label)
{
    return fwd()->add_option(id, label);
}

std::list<FilterOption::SCPtr> OptionSelector::options() const
{
    return fwd()->options();
}

std::list<FilterOption::SCPtr> OptionSelector::active_options(FilterState const& filter_state) const
{
    return fwd()->active_options(filter_state);
}

void OptionSelector::update_state(FilterState& filter_state, FilterOption::SCPtr option, bool state) const
{
    fwd()->update_state(filter_state, option, state);
}

internal::OptionSelectorImpl* OptionSelector::fwd() const
{
    return dynamic_cast<internal::OptionSelectorImpl*>(p.get());
}

} // namespace scopes

} // namespace api

} // namespace unity

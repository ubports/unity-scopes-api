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

#include <scopes/internal/OptionSelectorImpl.h>
#include <scopes/FilterState.h>

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{
    
OptionSelectorImpl::OptionSelectorImpl(std::string const& id, std::string const& label, bool multi_select)
    : FilterBaseImpl(id),
    label_(label),
    multi_select_(multi_select)
{
}

std::string OptionSelectorImpl::label() const
{
    return label_;
}

bool OptionSelectorImpl::multi_select() const
{
    return multi_select_;
}

FilterOption::SCPtr OptionSelectorImpl::add_option(std::string const& id, std::string const& label)
{
    auto opt = std::shared_ptr<FilterOption>(new FilterOption(id, label));
    options_.push_back(opt);
    return opt;
}

std::list<FilterOption::SCPtr> OptionSelectorImpl::options() const
{
    return options_;
}

std::list<FilterOption::SCPtr> OptionSelectorImpl::active_options(FilterState const& filter_state) const
{
    std::list<FilterOption::SCPtr> opts;
    return opts;
}

void OptionSelectorImpl::update_state(FilterState& filter_state, FilterOption::SCPtr option, bool state) const
{
    //TODO ensure this option belongs to this filter?
    filter_state.set_option_selector_value(id(), option->id(), state);
}

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

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

#include <scopes/internal/OptionSelectorFilterImpl.h>
#include <scopes/FilterState.h>
#include <unity/UnityExceptions.h>
#include <unordered_set>
#include <algorithm>

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

OptionSelectorFilterImpl::OptionSelectorFilterImpl(std::string const& id, std::string const& label, bool multi_select)
    : FilterBaseImpl(id),
    label_(label),
    multi_select_(multi_select)
{
}

std::string OptionSelectorFilterImpl::label() const
{
    return label_;
}

bool OptionSelectorFilterImpl::multi_select() const
{
    return multi_select_;
}

FilterOption::SCPtr OptionSelectorFilterImpl::add_option(std::string const& id, std::string const& label)
{
    auto opt = std::shared_ptr<FilterOption>(new FilterOption(id, label));
    options_.push_back(opt);
    return opt;
}

std::list<FilterOption::SCPtr> OptionSelectorFilterImpl::options() const
{
    return options_;
}

std::set<FilterOption::SCPtr> OptionSelectorFilterImpl::active_options(FilterState const& filter_state) const
{
    std::set<FilterOption::SCPtr> opts;
    if (filter_state.has_filter(id()))
    {
        try
        {
            auto var = FilterBaseImpl::get(filter_state, id()).get_array(); // this can throw if of different type
            std::unordered_set<std::string> opts_ids;

            // create set for faster lookup
            for (auto const& idvar: var)
            {
                opts_ids.insert(idvar.get_string());
            }

            // insert selected options into opts
            for (auto const& opt: options_)
            {
                if (opts_ids.find(opt->id()) != opts_ids.end())
                {
                    opts.insert(opt);
                }
            }
        }
        catch (...)
        {
            // ignore all errors here - we could be getting an incorrect value for this filter
            // via a canned query from another scope, we shouldn't break this scope on it.
        }
    }
    return opts;
}

void OptionSelectorFilterImpl::update_state(FilterState& filter_state, FilterOption::SCPtr option, bool state) const
{
    auto const oid(option->id());
    auto it = std::find_if(options_.begin(), options_.end(), [oid](FilterOption::SCPtr const& opt) { return opt->id() == oid; });
    if (it == options_.end())
    {
        throw unity::InvalidArgumentException("OptionSelector::update_state(): unknown filter option: " + oid);
    }
    filter_state.set_option_selector_value(id(), option->id(), state);
}

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

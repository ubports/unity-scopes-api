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

#include <scopes/internal/FilterStateImpl.h>
#include <unity/UnityExceptions.h>
#include <algorithm>

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

bool FilterStateImpl::has_filter(std::string const& id) const
{
    return state_.find(id) != state_.end();
}

void FilterStateImpl::reset(std::string const& id)
{
    state_.erase(id);
}

Variant FilterStateImpl::get(std::string const& filter_id)
{
    auto it = state_.find(filter_id);
    if (it != state_.end())
    {
        return it->second;
    }
    throw InvalidArgumentException("Unknown fiter: " + filter_id);
}

void FilterStateImpl::set_option_selector_value(std::string const& filter_id, std::string const& option_id, bool value)
{
    auto it = state_.find(filter_id);
    // do we have this filter already?
    if (it == state_.end())
    {
        if (value)
        {
            state_[filter_id] = VariantArray({Variant(option_id)});
        }
        else
        {
            state_[filter_id] = VariantArray(); // no option active
        }
    }
    else // modify existing entry for this filter
    {
        VariantArray var = (it->second).get_array(); // may throw if this filter if this id was used for different filter type

        // do we have this option already?
        auto opt_it = std::find_if(var.begin(), var.end(), [option_id](Variant const& v1) { return v1.get_string() == option_id; });
        if (opt_it == var.end())
        {
            if (value)
            {
                var.push_back(Variant(option_id));
            } // else - option not selected, nothing to do
        }
        else // option already stored in the state
        {
            if (!value) // remove if it's now unselected
            {
                var.erase(opt_it);
            }
        }
        state_[filter_id] = std::move(var);
    }
}

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

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

#include <unity/scopes/internal/FilterStateImpl.h>
#include <unity/UnityExceptions.h>
#include <unity/scopes/FilterState.h>
#include <algorithm>

namespace unity
{

namespace scopes
{

namespace internal
{

FilterStateImpl::FilterStateImpl(VariantMap const& var)
    : state_(var)
{
}

bool FilterStateImpl::has_filter(std::string const& id) const
{
    return state_.find(id) != state_.end();
}

void FilterStateImpl::remove(std::string const& id)
{
    state_.erase(id);
}

Variant FilterStateImpl::get(std::string const& filter_id) const
{
    auto it = state_.find(filter_id);
    if (it != state_.end())
    {
        return it->second;
    }
    throw InvalidArgumentException("Unknown fiter: " + filter_id);
}

VariantMap& FilterStateImpl::get()
{
    return state_;
}

VariantMap FilterStateImpl::serialize() const
{
    return state_;
}

FilterState FilterStateImpl::deserialize(VariantMap const& var)
{
    return FilterState(new FilterStateImpl(var));
}

} // namespace internal

} // namespace scopes

} // namespace unity

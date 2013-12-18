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

#include <scopes/FilterState.h>
#include <unity/UnityExceptions.h>
#include <scopes/internal/FilterStateImpl.h>

namespace unity
{

namespace api
{

namespace scopes
{

FilterState::FilterState()
    : p(new internal::FilterStateImpl())
{
}

FilterState::FilterState(FilterState const& other)
    : p(new internal::FilterStateImpl(*(other.p)))
{
}

FilterState::FilterState(internal::FilterStateImpl *pimpl)
    : p(pimpl)
{
}

FilterState::FilterState(FilterState &&) = default;
FilterState& FilterState::operator=(FilterState &&) = default;

FilterState& FilterState::operator=(FilterState const& other)
{
    if (this != &other)
    {
        p.reset(new internal::FilterStateImpl(*(other.p)));
    }
    return *this;
}

bool FilterState::has_filter(std::string const& id) const
{
    return p->has_filter(id);
}

void FilterState::remove(std::string const& id)
{
    p->remove(id);
}

} // namespace scopes

} // namespace api

} // namespace unity

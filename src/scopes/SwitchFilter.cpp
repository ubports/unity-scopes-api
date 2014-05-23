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

#include <unity/scopes/SwitchFilter.h>
#include <unity/scopes/internal/SwitchFilterImpl.h>

namespace unity
{

namespace scopes
{

SwitchFilter::UPtr SwitchFilter::create(std::string const& id, std::string const& label)
{
    return std::unique_ptr<SwitchFilter>(new SwitchFilter(new internal::SwitchFilterImpl(id, label)));
}

std::string SwitchFilter::label() const
{
    return fwd()->label();
}

bool SwitchFilter::is_on(FilterState const& filter_state) const
{
    return fwd()->is_on(filter_state);
}

void SwitchFilter::update_state(FilterState& filter_state, bool active) const
{
    fwd()->update_state(filter_state, active);
}

void SwitchFilter::update_state(FilterState& filter_state, std::string const& filter_id, bool active)
{
    internal::SwitchFilterImpl::update_state(filter_state, filter_id, active);
}

SwitchFilter::SwitchFilter(internal::SwitchFilterImpl* impl)
    : FilterBase(impl)
{
}

internal::SwitchFilterImpl* SwitchFilter::fwd() const
{
    return dynamic_cast<internal::SwitchFilterImpl*>(p.get());
}

} // namespace scopes

} // namespace unity

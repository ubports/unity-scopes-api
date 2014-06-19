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

#include <unity/scopes/internal/SwitchFilterImpl.h>
#include <unity/scopes/internal/Utils.h>
#include <unity/scopes/FilterState.h>
#include <unity/UnityExceptions.h>

namespace unity
{

namespace scopes
{

namespace internal
{

SwitchFilterImpl::SwitchFilterImpl(std::string const& id, std::string const& label)
    : FilterBaseImpl(id),
      label_(label)
{
    if (label.empty())
    {
        throw InvalidArgumentException("SwitchFilterImpl(): Invalid empty label string");
    }
}

SwitchFilterImpl::SwitchFilterImpl(VariantMap const& var)
    : FilterBaseImpl(var)
{
    deserialize(var);
}

std::string SwitchFilterImpl::label() const
{
    return label_;
}

SwitchFilter::SPtr SwitchFilterImpl::create(VariantMap const& var)
{
    return std::shared_ptr<SwitchFilter>(new SwitchFilter(new SwitchFilterImpl(var)));
}

bool SwitchFilterImpl::is_on(FilterState const& filter_state) const
{
    if (filter_state.has_filter(id()))
    {
        try
        {
            return FilterBaseImpl::get(filter_state, id()).get_bool(); // this can throw if of different type
        }
        catch (...)
        {
        }
    }
    return false;
}

void SwitchFilterImpl::update_state(FilterState& filter_state, bool active) const
{
    update_state(filter_state, id(), active);
}

void SwitchFilterImpl::update_state(FilterState& filter_state, std::string const& filter_id, bool active)
{
    if (filter_id.empty())
    {
        throw InvalidArgumentException("SwitchFilterImpl::update_state(): Invalid empty filter_id string");
    }

    VariantMap& state = FilterBaseImpl::get(filter_state);
    state[filter_id] = Variant(active);
}

std::string SwitchFilterImpl::filter_type() const
{
    return "switch";
}

void SwitchFilterImpl::serialize(VariantMap& var) const
{
    var["label"] = label_;
}

void SwitchFilterImpl::deserialize(VariantMap const& var)
{
    auto it = find_or_throw("SwitchFilterImpl::deserialize()", var, "label");
    label_ = it->second.get_string();
}

} // namespace internal

} // namespace scopes

} // namespace unity

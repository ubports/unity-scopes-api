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

#include <unity/scopes/ActivationQueryBase.h>
#include <unity/scopes/internal/ActivationQueryBaseImpl.h>

namespace unity
{

namespace scopes
{

/// @cond
ActivationQueryBase::ActivationQueryBase(Result const& result, ActionMetadata const& metadata) :
    QueryBase(new internal::ActivationQueryBaseImpl(result, metadata))
{
}


ActivationQueryBase::ActivationQueryBase(Result const& result, ActionMetadata const& metadata, std::string const& widget_id, std::string const& action_id) :
    QueryBase(new internal::ActivationQueryBaseImpl(result, metadata, widget_id, action_id))
{
}

ActivationQueryBase::ActivationQueryBase(Result const& update_result, ActionMetadata const& metadata, std::string const& action_id) :
    QueryBase(new internal::ActivationQueryBaseImpl(update_result, metadata, "", action_id))
{
}

ActivationQueryBase::~ActivationQueryBase()
{
}
/// @endcond

void ActivationQueryBase::cancelled()
{
    //default implementation does nothing
}

ActivationResponse ActivationQueryBase::activate()
{
    return fwd()->activate();
}

Result ActivationQueryBase::result() const
{
    return fwd()->result();
}

ActionMetadata ActivationQueryBase::action_metadata() const
{
    return fwd()->action_metadata();
}

std::string ActivationQueryBase::widget_id() const
{
    return fwd()->widget_id();
}

std::string ActivationQueryBase::action_id() const
{
    return fwd()->action_id();
}

internal::ActivationQueryBaseImpl* ActivationQueryBase::fwd() const
{
    return dynamic_cast<internal::ActivationQueryBaseImpl*>(p.get());
}

} // namespace scopes

} // namespace unity

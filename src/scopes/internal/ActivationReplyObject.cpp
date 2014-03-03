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

#include <unity/scopes/internal/ActivationReplyObject.h>
#include <unity/scopes/ActivationResponse.h>
#include <unity/scopes/internal/ActivationResponseImpl.h>
#include <cassert>

namespace unity
{

namespace scopes
{

namespace internal
{

ActivationReplyObject::ActivationReplyObject(ActivationListenerBase::SPtr const& receiver, RuntimeImpl const* runtime, std::string const& scope_name) :
    ReplyObject(std::static_pointer_cast<ListenerBase>(receiver), runtime, scope_name),
    receiver_(receiver)
{
    assert(receiver_);
}

bool ActivationReplyObject::process_data(VariantMap const& data)
{
    ActivationResponse resp = ActivationResponseImpl::create(data);
    receiver_->activated(resp);
    return false;
}

} // namespace internal

} // namespace scopes

} // namespace unity

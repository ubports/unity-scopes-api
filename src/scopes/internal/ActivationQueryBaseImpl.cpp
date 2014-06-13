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

#include <unity/scopes/internal/ActivationQueryBaseImpl.h>

namespace unity
{

namespace scopes
{

namespace internal
{

using namespace std;

ActivationQueryBaseImpl::ActivationQueryBaseImpl(Result const& result, ActionMetadata const& metadata, std::string const& widget_id,
        std::string const& action_id) :
    QueryBaseImpl(),
    result_(result),
    metadata_(metadata),
    widget_id_(widget_id),
    action_id_(action_id),
    valid_(true)
{
}

ActivationResponse ActivationQueryBaseImpl::activate()
{
    return ActivationResponse(ActivationResponse::Status::NotHandled);
}

void ActivationQueryBaseImpl::cancel()
{
    lock_guard<mutex> lock(mutex_);
    valid_ = false;
}

bool ActivationQueryBaseImpl::valid() const
{
    lock_guard<mutex> lock(mutex_);
    return valid_;
}

Result ActivationQueryBaseImpl::result() const
{
    return result_;
}

ActionMetadata ActivationQueryBaseImpl::action_metadata() const
{
    return metadata_;
}

std::string ActivationQueryBaseImpl::widget_id() const
{
    return widget_id_;
}

std::string ActivationQueryBaseImpl::action_id() const
{
    return action_id_;
}

} // namespace internal

} // namespace scopes

} // namespace unity

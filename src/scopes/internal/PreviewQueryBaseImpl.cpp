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

#include <unity/scopes/internal/PreviewQueryBaseImpl.h>

namespace unity
{

namespace scopes
{

namespace internal
{

using namespace std;

PreviewQueryBaseImpl::PreviewQueryBaseImpl(Result const& result, ActionMetadata const& metadata)
    : QueryBaseImpl(),
      valid_(true),
      result_(result),
      action_metadata_(metadata)
{
}

Result PreviewQueryBaseImpl::result() const
{
    return result_;
}

ActionMetadata PreviewQueryBaseImpl::action_metadata() const
{
    return action_metadata_;
}

void PreviewQueryBaseImpl::cancel()
{
    lock_guard<mutex> lock(mutex_);
    valid_ = false;
}

bool PreviewQueryBaseImpl::valid() const
{
    lock_guard<mutex> lock(mutex_);
    return valid_;
}

} // namespace internal

} // namespace scopes

} // namespace unity

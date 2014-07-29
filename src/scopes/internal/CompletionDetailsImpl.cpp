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
 * Authored by: Marcus Tomlinson <marcus.tomlinson@canonical.com>
*/

#include <unity/scopes/internal/CompletionDetailsImpl.h>

namespace unity
{

namespace scopes
{

namespace internal
{

CompletionDetailsImpl::CompletionDetailsImpl(CompletionDetails::CompletionStatus status)
    : status_(status)
{
}

CompletionDetailsImpl::CompletionDetailsImpl(CompletionDetails::CompletionStatus status, std::string const& message)
    : status_(status)
    , message_(message)
{
}

CompletionDetails::CompletionStatus CompletionDetailsImpl::status() const noexcept
{
    return status_;
}

std::string CompletionDetailsImpl::message() const
{
    return message_;
}

void CompletionDetailsImpl::add_info(OperationInfo const& info)
{
    info_list_.push_back(info);
}

std::vector<OperationInfo> CompletionDetailsImpl::info_list() const
{
    return info_list_;
}

} // namespace internal

} // namespace scopes

} // namespace unity

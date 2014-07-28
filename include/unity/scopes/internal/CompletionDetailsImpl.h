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

#ifndef UNITY_INTERNAL_COMPLETIONDETAILSIMPL_H
#define UNITY_INTERNAL_COMPLETIONDETAILSIMPL_H

#include <unity/scopes/CompletionDetails.h>

namespace unity
{

namespace scopes
{

namespace internal
{

class CompletionDetailsImpl final
{
public:
    CompletionDetailsImpl(CompletionDetails::CompletionStatus status);
    CompletionDetailsImpl(CompletionDetails::CompletionStatus status, std::list<OperationInfo> const& details);

    CompletionDetailsImpl(CompletionDetailsImpl const&) = default;
    CompletionDetailsImpl(CompletionDetailsImpl&&) = default;

    CompletionDetailsImpl& operator=(CompletionDetailsImpl const&) = default;
    CompletionDetailsImpl& operator=(CompletionDetailsImpl&&) = default;

    CompletionDetails::CompletionStatus status() const noexcept;
    std::list<OperationInfo> details() const noexcept;

private:
    CompletionDetails::CompletionStatus status_;
    std::list<OperationInfo> details_;
};

} // namespace internal

} // namespace scopes

} // namespace unity

#endif

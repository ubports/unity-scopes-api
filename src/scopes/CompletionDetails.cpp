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

#include <unity/scopes/CompletionDetails.h>
#include <unity/scopes/internal/CompletionDetailsImpl.h>

#include <assert.h>
#include <unordered_map>

namespace unity
{

namespace scopes
{

CompletionDetails::CompletionDetails(CompletionStatus status)
    : p(new internal::CompletionDetailsImpl(status))
{
}

CompletionDetails::CompletionDetails(CompletionStatus status, std::string const& message)
    : p(new internal::CompletionDetailsImpl(status, message))
{
}

/// @cond

CompletionDetails::CompletionDetails(CompletionDetails const& other)
    : p(new internal::CompletionDetailsImpl(*other.p.get()))
{
}

CompletionDetails::CompletionDetails(CompletionDetails&&) = default;

CompletionDetails& CompletionDetails::operator=(CompletionDetails const& other)
{
    if (this != &other)
    {
        p.reset(new internal::CompletionDetailsImpl(*other.p.get()));
    }
    return *this;
}

CompletionDetails& CompletionDetails::operator=(CompletionDetails&&) = default;

CompletionDetails::~CompletionDetails() = default;

/// @endcond

CompletionDetails::CompletionStatus CompletionDetails::status() const noexcept
{
    return p->status();
}

std::string CompletionDetails::message() const
{
    return p->message();
}

void CompletionDetails::add_info(OperationInfo const& info) noexcept
{
    p->add_info(info);
}

std::vector<OperationInfo> CompletionDetails::info_list() const noexcept
{
    return p->info_list();
}

// Possibly overkill, but safer than using the enum as the index into an array,
// in case the enumeration is ever added to or the enumerators get re-ordered.

static std::unordered_map<int, char const*> const statuses =
{
    std::pair<int, char const*>(static_cast<int>(CompletionDetails::OK), "ok"),
    std::pair<int, char const*>(static_cast<int>(CompletionDetails::Cancelled), "cancelled"),
    std::pair<int, char const*>(static_cast<int>(CompletionDetails::Error), "error")
};

char const* to_string(CompletionDetails::CompletionStatus status)
{
    assert(statuses.find(static_cast<int>(status)) != statuses.end());
    return statuses.find(static_cast<int>(status))->second;
}

} // namespace scopes

} // namespace unity

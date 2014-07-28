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

#include <unity/scopes/OperationInfo.h>
#include <unity/scopes/internal/OperationInfoImpl.h>

namespace unity
{

namespace scopes
{

OperationInfo::OperationInfo(InfoCode code)
    : p(new internal::OperationInfoImpl(code))
{
}

OperationInfo::OperationInfo(InfoCode code, std::string message)
    : p(new internal::OperationInfoImpl(code, message))
{
}

/// @cond

OperationInfo::OperationInfo(OperationInfo const& other)
    : p(new internal::OperationInfoImpl(*other.p.get()))
{
}

OperationInfo::OperationInfo(OperationInfo&&) = default;

OperationInfo& OperationInfo::operator=(OperationInfo const& other)
{
    if (this != &other)
    {
        p.reset(new internal::OperationInfoImpl(*other.p.get()));
    }
    return *this;
}

OperationInfo& OperationInfo::operator=(OperationInfo&&) = default;

OperationInfo::~OperationInfo() = default;
/// @endcond

OperationInfo::InfoCode OperationInfo::code() const noexcept
{
    return p->code();
}

std::string OperationInfo::message() const
{
    return p->message();
}

} // namespace scopes

} // namespace unity

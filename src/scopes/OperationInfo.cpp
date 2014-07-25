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

namespace unity
{

namespace scopes
{

OperationInfo::OperationInfo(InfoCode code)
    : code_(code)
{
}

OperationInfo::OperationInfo(InfoCode code, std::string message)
    : code_(code)
    , message_(message)
{
}

OperationInfo::~OperationInfo()
{
}

OperationInfo::InfoCode OperationInfo::code() const noexcept
{
    return code_;
}

std::string OperationInfo::message() const noexcept
{
    return message_;
}

} // namespace scopes

} // namespace unity

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

#pragma once

#include <unity/scopes/OperationInfo.h>

namespace unity
{

namespace scopes
{

namespace internal
{

class OperationInfoImpl final
{
public:
    OperationInfoImpl(OperationInfo::InfoCode code);
    OperationInfoImpl(OperationInfo::InfoCode code, std::string message);

    OperationInfoImpl(OperationInfoImpl const&) = default;
    OperationInfoImpl(OperationInfoImpl&&) = default;

    OperationInfoImpl& operator=(OperationInfoImpl const&) = default;
    OperationInfoImpl& operator=(OperationInfoImpl&&) = default;

    OperationInfo::InfoCode code() const noexcept;
    std::string message() const noexcept;

private:
    OperationInfo::InfoCode code_;
    std::string message_;
};

} // namespace internal

} // namespace scopes

} // namespace unity

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
 * Authored by: Michi Henning <michi.henning@canonical.com>
 */

#pragma once

#include <unity/scopes/Object.h>

#include <gmock/gmock.h>

namespace unity
{

namespace scopes
{

namespace testing
{

/// @cond

class MockObject : public virtual Object
{
public:
    MockObject() = default;

    MOCK_METHOD0(endpoint, std::string());
    MOCK_METHOD0(identity, std::string());
    MOCK_METHOD0(target_category, std::string());
    MOCK_METHOD0(timeout, int64_t());
    MOCK_METHOD0(to_string, std::string());
};

/// @endcond

} // namespace testing

} // namespace scopes

} // namespace unity

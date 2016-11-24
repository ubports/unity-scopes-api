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
 * Authored by: James Henstridge <james.henstridge@canonical.com>
 */

#pragma once

#include <unity/scopes/QueryCtrl.h>

#include <unity/scopes/testing/MockObject.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <gmock/gmock.h>
#pragma GCC diagnostic pop

namespace unity
{

namespace scopes
{

namespace testing
{

/// @cond

class MockQueryCtrl : public unity::scopes::QueryCtrl, public unity::scopes::testing::MockObject
{
public:
    MockQueryCtrl() = default;

    MOCK_METHOD0(cancel, void());
};

/// @endcond

} // namespace testing

} // namespace scopes

} // namespace unity

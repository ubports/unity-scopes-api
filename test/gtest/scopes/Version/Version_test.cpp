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
 * Authored by: Michi Henning <michi.henning@canonical.com>
 */

#include <unity/scopes/Version.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <gtest/gtest.h>
#pragma GCC diagnostic pop

using namespace unity::scopes;

TEST(Version, basic)
{
    EXPECT_EQ(major_version(), UNITY_SCOPES_VERSION_MAJOR);
    EXPECT_EQ(minor_version(), UNITY_SCOPES_VERSION_MINOR);
    EXPECT_EQ(micro_version(), UNITY_SCOPES_VERSION_MICRO);
    EXPECT_STREQ(version_string(), UNITY_SCOPES_VERSION_STRING);
}

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

#include <unity/scopes/internal/UniqueID.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <gtest/gtest.h>
#pragma GCC diagnostic pop

using namespace std;
using namespace unity::scopes::internal;

bool has_suffix(string const& s, string const& suffix)
{
    auto s_len = s.length();
    auto suffix_len = suffix.length();
    if (s_len >= suffix_len)
    {
        return s.compare(s_len - suffix_len, suffix_len, suffix) == 0;
    }
    return false;
}

TEST(UniqueID, basic)
{
    UniqueID u;

    string id = u.gen();
    EXPECT_EQ(16u, id.size());
    EXPECT_TRUE(has_suffix(id, "00000000"));

    string id2 = u.gen();
    EXPECT_TRUE(has_suffix(id2, "00000001"));

    UniqueID v;
    id = v.gen();
    EXPECT_TRUE(has_suffix(id, "00000000"));    // New generator starts new sequence
}

TEST(UniqueID, seed)
{
    UniqueID u(1);
    UniqueID v(1);

    string id = u.gen();
    string id2 = v.gen();
    EXPECT_EQ(id, id2);
}

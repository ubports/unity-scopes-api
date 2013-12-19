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
 * Authored by: Pawel Stolowski <pawel.stolowski@canonical.com>
 */

#include <scopes/internal/CategoryRegistry.h>
#include <scopes/CategoryRenderer.h>
#include <unity/UnityExceptions.h>
#include <scopes/Variant.h>

#include <gtest/gtest.h>

using namespace std;
using namespace unity;
using namespace unity::scopes;
using namespace unity::scopes::internal;

TEST(CategoryRegistry, basic)
{
    CategoryRegistry reg;
    {
        CategoryRenderer rdr;
        EXPECT_EQ(nullptr, reg.lookup_category("a"));
        auto cat = reg.register_category("a", "title", "icon", rdr);
        EXPECT_TRUE(cat != nullptr);

        auto cat1 = reg.lookup_category("a");
        EXPECT_TRUE(cat1 != nullptr);
        EXPECT_TRUE(cat == cat1);
    }
 }

TEST(CategoryRegistry, exceptions)
{
    CategoryRegistry reg;
    CategoryRenderer rdr;

    auto cat = reg.register_category("a", "title", "icon", rdr);
    EXPECT_THROW(reg.register_category("a", "title1", "icon1", rdr), InvalidArgumentException);
    EXPECT_THROW(reg.register_category(cat), InvalidArgumentException);
}

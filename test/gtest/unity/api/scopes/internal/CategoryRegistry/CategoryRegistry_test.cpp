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
#include <unity/UnityExceptions.h>
#include <scopes/Variant.h>

#include <gtest/gtest.h>

using namespace std;
using namespace unity;
using namespace unity::api::scopes;
using namespace unity::api::scopes::internal;

TEST(CategoryRegistry, basic)
{
    CategoryRegistry reg;
    {
        EXPECT_EQ(nullptr, reg.lookup_category("a"));
        auto cat = reg.register_category("a", "title", "icon", "{}");
        EXPECT_TRUE(cat != nullptr);

        auto cat1 = reg.lookup_category("a");
        EXPECT_TRUE(cat1 != nullptr);
        EXPECT_TRUE(cat == cat1);
    }

    {
        VariantMap vm;
        vm["id"] = "b";
        vm["title"] = "title";
        vm["icon"] = "icon";
        vm["renderer_template"] = "{\"a\":1}";

        auto cat = reg.register_category(vm);
        EXPECT_TRUE(cat != nullptr);
        EXPECT_EQ("b", cat->id());
        EXPECT_EQ("title", cat->title());
        EXPECT_EQ("icon", cat->icon());
        EXPECT_EQ("{\"a\":1}", cat->renderer_template());
    }
}

TEST(CategoryRegistry, exceptions)
{
    CategoryRegistry reg;

    auto cat = reg.register_category("a", "title", "icon", "{}");
    EXPECT_THROW(reg.register_category("a", "title1", "icon1", "{}"), InvalidArgumentException);
    EXPECT_THROW(reg.register_category(cat), InvalidArgumentException);
}

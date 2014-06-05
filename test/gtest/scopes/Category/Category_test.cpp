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

#include <unity/scopes/internal/CategoryRegistry.h>
#include <unity/scopes/CategoryRenderer.h>
#include <unity/UnityExceptions.h>
#include <unity/scopes/Variant.h>

#include <gtest/gtest.h>

using namespace std;
using namespace unity;
using namespace unity::scopes;
using namespace unity::scopes::internal;

TEST(Category, basic)
{
    CategoryRegistry reg;
    CategoryRenderer rdr("{\"a\":1}");

    auto cat = reg.register_category("a", "title", "icon", rdr);

    EXPECT_TRUE(cat != nullptr);
    EXPECT_EQ("a", cat->id());
    EXPECT_EQ("title", cat->title());
    EXPECT_EQ("icon", cat->icon());
    EXPECT_EQ("{\"a\":1}", cat->renderer_template().data());
    EXPECT_EQ(Category::TapBehavior::ShowPreview, cat->single_tap_behavior());
    EXPECT_EQ(Category::TapBehavior::ShowPreview, cat->long_tap_behavior());
}

TEST(Category, serialize)
{
    CategoryRegistry reg;
    CategoryRenderer rdr("{\"a\":1}");
    {
        auto cat = reg.register_category("a", "title", "icon", rdr);
        auto vm = cat->serialize();
        EXPECT_EQ("a", vm["id"].get_string());
        EXPECT_EQ("title", vm["title"].get_string());
        EXPECT_EQ("icon", vm["icon"].get_string());
        EXPECT_EQ("{\"a\":1}", vm["renderer_template"].get_string());
        EXPECT_EQ(Category::TapBehavior::ShowPreview, static_cast<Category::TapBehavior>(vm["single_tap_behavior"].get_int()));
        EXPECT_EQ(Category::TapBehavior::ShowPreview, static_cast<Category::TapBehavior>(vm["long_tap_behavior"].get_int()));
    }
}

TEST(Category, deserialize)
{
    VariantMap vm;
    vm["id"] = "b";
    vm["title"] = "title";
    vm["icon"] = "icon";
    vm["renderer_template"] = "{\"a\":1}";
    vm["single_tap_behavior"] = static_cast<int>(Category::TapBehavior::ActivateResult);
    vm["long_tap_behavior"] = static_cast<int>(Category::TapBehavior::Ignore);

    CategoryRegistry reg;
    auto cat = reg.register_category(vm);
    EXPECT_TRUE(cat != nullptr);
    EXPECT_EQ("b", cat->id());
    EXPECT_EQ("title", cat->title());
    EXPECT_EQ("icon", cat->icon());
    EXPECT_EQ("{\"a\":1}", cat->renderer_template().data());
    EXPECT_EQ(Category::TapBehavior::Ignore, static_cast<Category::TapBehavior>(cat->long_tap_behavior()));
    EXPECT_EQ(Category::TapBehavior::ActivateResult, static_cast<Category::TapBehavior>(cat->single_tap_behavior()));
}

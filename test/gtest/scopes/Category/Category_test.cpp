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

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <gtest/gtest.h>
#pragma GCC diagnostic pop

using namespace std;
using namespace unity;
using namespace unity::scopes;
using namespace unity::scopes::internal;

TEST(Category, basic)
{
    CategoryRegistry reg;
    CategoryRenderer rdr("{\"a\":1}");

    auto cat = reg.register_category("a", "title", "icon", nullptr, rdr);

    EXPECT_TRUE(cat != nullptr);
    EXPECT_EQ("a", cat->id());
    EXPECT_EQ("title", cat->title());
    EXPECT_EQ("icon", cat->icon());
    EXPECT_EQ("{\"a\":1}", cat->renderer_template().data());
    EXPECT_EQ(nullptr, cat->query().get());
}

TEST(Category, with_expansion_query)
{
    CategoryRegistry reg;
    CategoryRenderer rdr("{\"a\":1}");

    auto cat = reg.register_category("a", "title", "icon", make_shared<CannedQuery>("test-scope", "test-query", "dep-id"), rdr);

    EXPECT_TRUE(cat != nullptr);
    EXPECT_EQ("a", cat->id());
    EXPECT_EQ("title", cat->title());
    EXPECT_EQ("icon", cat->icon());
    EXPECT_EQ("{\"a\":1}", cat->renderer_template().data());
    EXPECT_NE(nullptr, cat->query().get());

    CannedQuery query(*(cat->query().get()));
    EXPECT_EQ("test-scope", query.scope_id());
    EXPECT_EQ("test-query", query.query_string());
    EXPECT_EQ("dep-id", query.department_id());
}

TEST(Category, serialize)
{
    CategoryRegistry reg;
    CategoryRenderer rdr("{\"a\":1}");
    {
        auto cat = reg.register_category("a", "title", "icon", nullptr, rdr);
        auto vm = cat->serialize();
        EXPECT_EQ("a", vm["id"].get_string());
        EXPECT_EQ("title", vm["title"].get_string());
        EXPECT_EQ("icon", vm["icon"].get_string());
        EXPECT_EQ("{\"a\":1}", vm["renderer_template"].get_string());
    }

    {
        CannedQuery q("test-scope");
        auto cat = reg.register_category("b", "title", "icon", make_shared<CannedQuery>(q), rdr);
        auto vm = cat->serialize();
        EXPECT_EQ("b", vm["id"].get_string());
        EXPECT_EQ("title", vm["title"].get_string());
        EXPECT_EQ("icon", vm["icon"].get_string());
        EXPECT_EQ("{\"a\":1}", vm["renderer_template"].get_string());
        EXPECT_EQ(q.to_uri(), vm["query"].get_string());
    }
}

TEST(Category, deserialize)
{
    VariantMap vm;
    vm["id"] = "b";
    vm["title"] = "title";
    vm["icon"] = "icon";
    vm["renderer_template"] = "{\"a\":1}";

    CategoryRegistry reg;
    auto cat = reg.register_category(vm);
    EXPECT_TRUE(cat != nullptr);
    EXPECT_EQ("b", cat->id());
    EXPECT_EQ("title", cat->title());
    EXPECT_EQ("icon", cat->icon());
    EXPECT_EQ("{\"a\":1}", cat->renderer_template().data());
}

TEST(Category, deserialize_with_query)
{
    VariantMap vm;
    vm["id"] = "b";
    vm["title"] = "title";
    vm["icon"] = "icon";
    vm["renderer_template"] = "{\"a\":1}";
    vm["query"] = "scope://test-scope";

    CategoryRegistry reg;
    auto cat = reg.register_category(vm);
    EXPECT_TRUE(cat != nullptr);
    EXPECT_EQ("b", cat->id());
    EXPECT_EQ("title", cat->title());
    EXPECT_EQ("icon", cat->icon());
    EXPECT_EQ("{\"a\":1}", cat->renderer_template().data());
    EXPECT_NE(nullptr, cat->query().get());
    EXPECT_EQ("test-scope", cat->query()->scope_id());
}

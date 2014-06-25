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
 * Authored by: Pawel Stolowski <pawel.stolowski@canonical.com>
 */

#include <unity/scopes/internal/ScopeConfig.h>
#include <unity/UnityExceptions.h>
#include <unity/scopes/ScopeExceptions.h>
#include <gtest/gtest.h>

using namespace unity::scopes;
using namespace unity::scopes::internal;

TEST(ScopeConfig, basic)
{
    {
        ScopeConfig cfg(TEST_CONFIG_1);

        EXPECT_EQ("Scope name", cfg.display_name());
        EXPECT_EQ("Scope description", cfg.description());
        EXPECT_EQ("scope art", cfg.art());
        EXPECT_EQ("Canonical", cfg.author());
        EXPECT_EQ("an icon", cfg.icon());
        EXPECT_EQ("search string", cfg.search_hint());
        EXPECT_EQ("a key", cfg.hot_key());
        EXPECT_EQ(ScopeMetadata::ResultsTtlType::None, cfg.results_ttl_type());

        auto attrs = cfg.appearance_attributes();
        EXPECT_EQ(4, attrs.size());
        EXPECT_EQ("foo", attrs["arbitrary_key"].get_string());
        EXPECT_EQ("bar", attrs["another_one"].get_string());
        EXPECT_TRUE(attrs.find("logo") != attrs.end());
        EXPECT_EQ(Variant::Type::Dict, attrs["logo"].which());
        EXPECT_EQ("first", attrs["logo"].get_dict()["one"].get_string());
        EXPECT_EQ("second", attrs["logo"].get_dict()["two"].get_string());
        EXPECT_EQ("abc", attrs["page"].get_dict()["header"].get_dict()["logo"].get_string());

        EXPECT_THROW(cfg.scope_runner(), unity::scopes::NotFoundException);
    }
    {
        ScopeConfig cfg(TEST_CONFIG_2);

        EXPECT_EQ("Scope name", cfg.display_name());
        EXPECT_EQ("Scope description", cfg.description());
        EXPECT_EQ("scope art", cfg.art());
        EXPECT_EQ("Canonical", cfg.author());
        EXPECT_EQ("an icon", cfg.icon());
        EXPECT_EQ("a search hint string", cfg.search_hint());
        EXPECT_EQ("a key", cfg.hot_key());
        EXPECT_EQ(ScopeMetadata::ResultsTtlType::None, cfg.results_ttl_type());

        EXPECT_EQ(0, cfg.appearance_attributes().size());
    }
    {
        ScopeConfig cfg(TEST_CONFIG_3);

        EXPECT_EQ("Scope name", cfg.display_name());
        EXPECT_EQ("Scope description", cfg.description());
        EXPECT_EQ("scope art", cfg.art());
        EXPECT_EQ("Canonical", cfg.author());
        EXPECT_EQ("an icon", cfg.icon());
        EXPECT_EQ("a search hint string", cfg.search_hint());
        EXPECT_EQ("a key", cfg.hot_key());
        EXPECT_EQ("/my/scope/runner", cfg.scope_runner());
        EXPECT_EQ(ScopeMetadata::ResultsTtlType::Small, cfg.results_ttl_type());

        EXPECT_EQ(0, cfg.appearance_attributes().size());
    }
}

class ScopeConfigWithIntl: public ::testing::Test
{
public:
    ScopeConfigWithIntl()
    {
        setenv("LANGUAGE", "test", 1);
    }

    ~ScopeConfigWithIntl()
    {
        unsetenv("LANGUAGE");
    }
};

TEST_F(ScopeConfigWithIntl, localization)
{
    {
        ScopeConfig cfg("configtest1.ini");

        EXPECT_EQ("copesay amenay", cfg.display_name());
        EXPECT_EQ("copesay criptiondesay", cfg.description());
        EXPECT_EQ("scope art", cfg.art());
        EXPECT_EQ("Canonical", cfg.author());
        EXPECT_EQ("an icon", cfg.icon());
        EXPECT_EQ("earchsay ringstay", cfg.search_hint());
        EXPECT_EQ("a key", cfg.hot_key());

        auto attrs = cfg.appearance_attributes();
        EXPECT_EQ(4, attrs.size());
        EXPECT_EQ("foo", attrs["arbitrary_key"].get_string());
        EXPECT_EQ("bar", attrs["another_one"].get_string());

        EXPECT_THROW(cfg.scope_runner(), unity::scopes::NotFoundException);
    }
}

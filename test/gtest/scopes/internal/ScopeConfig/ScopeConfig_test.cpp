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

#include <unity/scopes/internal/DfltConfig.h>
#include <unity/scopes/ScopeExceptions.h>
#include <unity/UnityExceptions.h>

#include <boost/regex.hpp>  // Use Boost implementation until http://gcc.gnu.org/bugzilla/show_bug.cgi?id=53631 is fixed.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <gtest/gtest.h>
#pragma GCC diagnostic pop

using namespace unity::scopes;
using namespace unity::scopes::internal;

TEST(ScopeConfig, basic)
{
    {
        ScopeConfig cfg(COMPLETE_CONFIG);

        // All items, whether mandatory or optional

        EXPECT_TRUE(cfg.overrideable());
        EXPECT_EQ("Scope name", cfg.display_name());
        EXPECT_EQ("Scope description", cfg.description());
        EXPECT_EQ("scope art", cfg.art());
        EXPECT_EQ("Canonical", cfg.author());
        EXPECT_EQ("an icon", cfg.icon());
        EXPECT_EQ("search string", cfg.search_hint());
        EXPECT_EQ("a key", cfg.hot_key());
        EXPECT_TRUE(cfg.invisible());
        EXPECT_EQ("custom runner", cfg.scope_runner());
        EXPECT_EQ(300, cfg.idle_timeout());
        EXPECT_EQ(ScopeMetadata::ResultsTtlType::Large, cfg.results_ttl_type());
        EXPECT_TRUE(cfg.location_data_needed());
        EXPECT_TRUE(cfg.debug_mode());
        EXPECT_EQ(1, cfg.version());

        auto children = cfg.child_scope_ids();
        EXPECT_EQ(3u, children.size());
        EXPECT_EQ("com.foo.bar", children[0]);
        EXPECT_EQ("com.foo.bar2", children[1]);
        EXPECT_EQ("com.foo.boo", children[2]);

        auto keywords = cfg.keywords();
        EXPECT_EQ(2u, keywords.size());
        EXPECT_NE(keywords.end(), keywords.find("foo"));
        EXPECT_NE(keywords.end(), keywords.find("bar"));

        EXPECT_TRUE(cfg.is_aggregator());

        auto attrs = cfg.appearance_attributes();
        EXPECT_EQ(5u, attrs.size());
        EXPECT_TRUE(attrs["arbitrary_key"].get_bool());
        EXPECT_EQ("bar", attrs["another_one"].get_string());
        EXPECT_EQ(11, attrs["num_key"].get_int());
        EXPECT_TRUE(attrs.find("logo") != attrs.end());
        EXPECT_EQ(Variant::Type::Dict, attrs["logo"].which());
        EXPECT_EQ("first", attrs["logo"].get_dict()["one"].get_string());
        EXPECT_EQ("second", attrs["logo"].get_dict()["two"].get_string());
        EXPECT_EQ("abc", attrs["page"].get_dict()["page-header"].get_dict()["logo"].get_string());
        EXPECT_EQ("def", attrs["page"].get_dict()["page-header"].get_dict()["logo2"].get_string());

    }

    {
        ScopeConfig cfg(MANDATORY_CONFIG);

        // Mandatory items

        EXPECT_EQ("Scope name", cfg.display_name());
        EXPECT_EQ("Scope description", cfg.description());
        EXPECT_EQ("Canonical", cfg.author());

        // Optional items that have a default

        EXPECT_FALSE(cfg.invisible());
        EXPECT_EQ(DFLT_SCOPE_IDLE_TIMEOUT, cfg.idle_timeout());
        EXPECT_EQ(ScopeMetadata::ResultsTtlType::None, cfg.results_ttl_type());
        EXPECT_FALSE(cfg.location_data_needed());
        EXPECT_FALSE(cfg.debug_mode());
        EXPECT_EQ(0, cfg.version());
        EXPECT_FALSE(cfg.is_aggregator());

        EXPECT_EQ(0u, cfg.appearance_attributes().size());

        // Optional items that throw if not set

        EXPECT_THROW(cfg.art(), NotFoundException);
        EXPECT_THROW(cfg.icon(), NotFoundException);
        EXPECT_THROW(cfg.search_hint(), NotFoundException);
        EXPECT_THROW(cfg.hot_key(), NotFoundException);
        EXPECT_THROW(cfg.scope_runner(), NotFoundException);
    }

    // These two are needed to get 100% coverage.
    {
        ScopeConfig cfg(TTL_SMALL);
        EXPECT_EQ(ScopeMetadata::ResultsTtlType::Small, cfg.results_ttl_type());
    }

    {
        ScopeConfig cfg(TTL_MEDIUM);
        EXPECT_EQ(ScopeMetadata::ResultsTtlType::Medium, cfg.results_ttl_type());
    }
}

TEST(ScopeConfig, bad_child_scope_ids)
{
    try
    {
        ScopeConfig cfg(BAD_CHILD_IDS);
        FAIL();
    }
    catch(ConfigException const& e)
    {
        boost::regex r("unity::scopes::ConfigException: \".*\": Invalid empty scope id for ChildScopes");
        EXPECT_TRUE(boost::regex_match(e.what(), r));
    }
}

TEST(ScopeConfig, empty_scope_ids)
{
    ScopeConfig cfg(EMPTY_CHILD_IDS);
    EXPECT_EQ(0u, cfg.child_scope_ids().size());
}

TEST(ScopeConfig, single_scope_id)
{
    ScopeConfig cfg(SINGLE_CHILD_ID);
    EXPECT_EQ(1u, cfg.child_scope_ids().size());
}

TEST(ScopeConfig, bad_timeout)
{
    try
    {
        ScopeConfig cfg(BAD_TIMEOUT);
        FAIL();
    }
    catch(ConfigException const& e)
    {
        boost::regex r("unity::scopes::ConfigException: \".*\": Illegal value \\(-2\\) for IdleTimeout: "
                       "value must be >= 1 and <= [0-9]+");
        EXPECT_TRUE(boost::regex_match(e.what(), r));
    }
}

TEST(ScopeConfig, bad_ttl)
{
    try
    {
        ScopeConfig cfg(BAD_TTL);
        FAIL();
    }
    catch(ConfigException const& e)
    {
        boost::regex r("unity::scopes::ConfigException: \".*\": Illegal value \\(\"blah\"\\) for ResultsTtlType");
        EXPECT_TRUE(boost::regex_match(e.what(), r));
    }
}

TEST(ScopeConfig, bad_version)
{
    try
    {
        ScopeConfig cfg(BAD_VERSION);
        FAIL();
    }
    catch(ConfigException const& e)
    {
        boost::regex r("unity::scopes::ConfigException: \".*\": Version must be > 0");
        EXPECT_TRUE(boost::regex_match(e.what(), r));
    }
}

TEST(ScopeConfig, bad_keywords)
{
    try
    {
        ScopeConfig cfg(BAD_KEYWORDS);
        FAIL();
    }
    catch(ConfigException const& e)
    {
        boost::regex r("unity::scopes::ConfigException: \".*\": Invalid empty keyword string found in \"Keywords\" list");
        EXPECT_TRUE(boost::regex_match(e.what(), r));
    }
}

TEST(ScopeConfig, empty_keywords)
{
    ScopeConfig cfg(EMPTY_KEYWORDS);
    EXPECT_EQ(0u, cfg.keywords().size());
}

TEST(ScopeConfig, single_keyword)
{
    ScopeConfig cfg(SINGLE_KEYWORD);
    EXPECT_EQ(1u, cfg.keywords().size());
}

TEST(ScopeConfig, empty_display_name)
{
    try
    {
        ScopeConfig cfg(EMPTY_DISPLAY_NAME);
        FAIL();
    }
    catch(ConfigException const& e)
    {
        boost::regex r("unity::scopes::ConfigException: \".*\": DisplayName cannot be empty or whitespace only");
        EXPECT_TRUE(boost::regex_match(e.what(), r));
    }
}

TEST(ScopeConfig, empty_description)
{
    try
    {
        ScopeConfig cfg(EMPTY_DESCRIPTION);
        FAIL();
    }
    catch(ConfigException const& e)
    {
        boost::regex r("unity::scopes::ConfigException: \".*\": Description cannot be empty or whitespace only");
        EXPECT_TRUE(boost::regex_match(e.what(), r));
    }
}

TEST(ScopeConfig, empty_author)
{
    try
    {
        ScopeConfig cfg(EMPTY_AUTHOR);
        FAIL();
    }
    catch(ConfigException const& e)
    {
        boost::regex r("unity::scopes::ConfigException: \".*\": Author cannot be empty or whitespace only");
        EXPECT_TRUE(boost::regex_match(e.what(), r));
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
        ScopeConfig cfg(COMPLETE_CONFIG);

        EXPECT_EQ("copesay amenay", cfg.display_name());
        EXPECT_EQ("copesay criptiondesay", cfg.description());
        EXPECT_EQ("earchsay ringstay", cfg.search_hint());
    }
}

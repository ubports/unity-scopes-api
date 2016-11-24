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
 * along with this program.  If not, see <http://www.gnu.org/lzmqnses/>.
 *
 * Authored by: Michi Henning <michi.henning@canonical.com>
 */

#include <unity/scopes/internal/ScopeMetadataImpl.h>

#include <unity/scopes/internal/RuntimeImpl.h>
#include <unity/scopes/internal/ScopeImpl.h>
#include <unity/scopes/internal/zmq_middleware/ZmqMiddleware.h>
#include <unity/scopes/ScopeExceptions.h>
#include <unity/UnityExceptions.h>

#include <boost/regex.hpp>  // Use Boost implementation until http://gcc.gnu.org/bugzilla/show_bug.cgi?id=53631 is fixed.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <gtest/gtest.h>
#pragma GCC diagnostic pop

using namespace std;
using namespace unity;
using namespace unity::scopes;
using namespace unity::scopes::internal;
using namespace unity::scopes::internal::zmq_middleware;

string const runtime_ini = TEST_DIR "/Runtime.ini";
string const zmq_ini = TEST_DIR "/Zmq.ini";

TEST(ScopeMetadataImpl, basic)
{
    auto rt = RuntimeImpl::create("testscope", runtime_ini);
    ZmqMiddleware mw("testscope", rt.get(), zmq_ini);

    unique_ptr<ScopeMetadataImpl> mi(new ScopeMetadataImpl(&mw));
    mi->set_scope_id("scope_id");
    auto mw_proxy = mw.create_scope_proxy("identity", "endpoint");
    mi->set_proxy(ScopeImpl::create(mw_proxy, "scope_id"));
    mi->set_display_name("display_name");
    mi->set_description("description");
    mi->set_author("author");
    mi->set_results_ttl_type(ScopeMetadata::ResultsTtlType::Medium);

    // Keep a copy for tests below
    unique_ptr<ScopeMetadataImpl> mi2(new ScopeMetadataImpl(*mi));

    // Create the public instance and check that the values match
    auto m = ScopeMetadataImpl::create(move(mi));
    EXPECT_EQ("scope_id", m.scope_id());
    EXPECT_EQ("identity", m.proxy()->identity());
    EXPECT_EQ("endpoint", m.proxy()->endpoint());
    EXPECT_EQ("display_name", m.display_name());
    EXPECT_EQ("description", m.description());
    EXPECT_EQ("author", m.author());
    EXPECT_EQ(0u, m.appearance_attributes().size());
    EXPECT_EQ(ScopeMetadata::ResultsTtlType::Medium, m.results_ttl_type());

    // Check that optional fields that are not set throw
    try
    {
        m.art();
        FAIL();
    }
    catch (NotFoundException const& e)
    {
        EXPECT_STREQ("unity::scopes::NotFoundException: attribute not set (name = art)", e.what());
    }

    try
    {
        m.icon();
        FAIL();
    }
    catch (NotFoundException const& e)
    {
        EXPECT_STREQ("unity::scopes::NotFoundException: attribute not set (name = icon)", e.what());
    }

    try
    {
        m.search_hint();
        FAIL();
    }
    catch (NotFoundException const& e)
    {
        EXPECT_STREQ("unity::scopes::NotFoundException: attribute not set (name = search_hint)", e.what());
    }

    try
    {
        m.hot_key();
        FAIL();
    }
    catch (NotFoundException const& e)
    {
        EXPECT_STREQ("unity::scopes::NotFoundException: attribute not set (name = hot_key)", e.what());
    }

    try
    {
        m.scope_directory();
        FAIL();
    }
    catch (NotFoundException const& e)
    {
        EXPECT_STREQ("unity::scopes::NotFoundException: attribute not set (name = scope_directory)", e.what());
    }

    try
    {
        m.settings_definitions();
        FAIL();
    }
    catch (NotFoundException const& e)
    {
        EXPECT_STREQ("unity::scopes::NotFoundException: attribute not set (name = settings_definitions)", e.what());
    }

    // when "invisible" is not set, false is returned
    EXPECT_FALSE(m.invisible());

    // when "location_data_needed" is not set, false is returned
    EXPECT_FALSE(m.location_data_needed());

    // when "version" is not set, 0 is returned
    EXPECT_EQ(0, m.version());

    // when "is_aggregator" is not set, false is returned
    EXPECT_FALSE(m.is_aggregator());

    // Check that the copy has the same values as the original
    EXPECT_EQ("scope_id", mi2->scope_id());
    EXPECT_EQ("identity", mi2->proxy()->identity());
    EXPECT_EQ("endpoint", mi2->proxy()->endpoint());
    EXPECT_EQ("display_name", mi2->display_name());
    EXPECT_EQ("description", mi2->description());
    EXPECT_EQ("author", mi2->author());
    EXPECT_EQ(0u, mi2->appearance_attributes().size());
    EXPECT_EQ(ScopeMetadata::ResultsTtlType::Medium, mi2->results_ttl_type());
    EXPECT_EQ(0, mi2->version());

    VariantMap attrs;
    attrs["foo"] = "bar";

    // Set optional fields on copy.
    mi2->set_art("art");
    mi2->set_icon("icon");
    mi2->set_search_hint("search_hint");
    mi2->set_hot_key("hot_key");
    mi2->set_invisible(true);
    mi2->set_appearance_attributes(attrs);
    mi2->set_scope_directory("/foo");
    mi2->set_results_ttl_type(ScopeMetadata::ResultsTtlType::Large);
    VariantArray va;
    va.push_back(Variant("hello"));
    mi2->set_settings_definitions(va);
    mi2->set_location_data_needed(true);
    mi2->set_child_scope_ids(vector<string>{"abc", "def"});
    mi2->set_keywords(set<string>{"music", "video", "news"});
    mi2->set_is_aggregator(true);

    // Make another copy, so we get coverage on the entire copy constructor
    unique_ptr<ScopeMetadataImpl> mi3(new ScopeMetadataImpl(*mi2));

    // Check that optional fields are set correctly
    m = ScopeMetadataImpl::create(move(mi2));
    EXPECT_EQ("search_hint", m.search_hint());
    EXPECT_EQ("hot_key", m.hot_key());
    EXPECT_TRUE(m.invisible());
    EXPECT_EQ("bar", m.appearance_attributes()["foo"].get_string());
    EXPECT_EQ(va, m.settings_definitions());
    EXPECT_TRUE(m.location_data_needed());
    EXPECT_EQ((vector<string>{"abc", "def"}), m.child_scope_ids());
    EXPECT_EQ((set<string>{"music", "video", "news"}), m.keywords());
    EXPECT_TRUE(m.is_aggregator());

    // Make another value
    unique_ptr<ScopeMetadataImpl> ti(new ScopeMetadataImpl(&mw));
    ti->set_scope_id("tmp scope_id");
    mw_proxy = mw.create_scope_proxy("tmp identity", "tmp endpoint");
    ti->set_proxy(ScopeImpl::create(mw_proxy, "tmp scope_id"));
    ti->set_display_name("tmp display_name");
    ti->set_description("tmp description");
    ti->set_author("tmp author");
    ti->set_art("tmp art");
    ti->set_icon("tmp icon");
    ti->set_search_hint("tmp search_hint");
    ti->set_hot_key("tmp hot_key");
    ti->set_invisible(true);
    ti->set_scope_directory("/foo");
    ti->set_appearance_attributes(attrs);
    ti->set_results_ttl_type(ScopeMetadata::ResultsTtlType::Small);
    ti->set_version(99);
    VariantArray tmp_va;
    tmp_va.push_back(Variant("tmp hello"));
    ti->set_settings_definitions(tmp_va);
    ti->set_location_data_needed(true);
    ti->set_child_scope_ids(vector<string>{"tmp abc", "tmp def"});
    ti->set_keywords(set<string>{"music", "video"});
    ti->set_is_aggregator(true);

    // Check impl assignment operator
    ScopeMetadataImpl ci(&mw);
    ci = *ti;
    EXPECT_EQ("tmp scope_id", ci.scope_id());
    EXPECT_EQ("tmp identity", ci.proxy()->identity());
    EXPECT_EQ("tmp endpoint", ci.proxy()->endpoint());
    EXPECT_EQ("tmp display_name", ci.display_name());
    EXPECT_EQ("tmp description", ci.description());
    EXPECT_EQ("tmp author", ci.author());
    EXPECT_EQ("tmp art", ci.art());
    EXPECT_EQ("tmp icon", ci.icon());
    EXPECT_EQ("tmp search_hint", ci.search_hint());
    EXPECT_EQ("tmp hot_key", ci.hot_key());
    EXPECT_EQ("/foo", ci.scope_directory());
    EXPECT_EQ("bar", ci.appearance_attributes()["foo"].get_string());
    EXPECT_TRUE(ci.invisible());
    EXPECT_EQ(ScopeMetadata::ResultsTtlType::Small, ci.results_ttl_type());
    EXPECT_EQ(tmp_va, ci.settings_definitions());
    EXPECT_TRUE(ci.location_data_needed());
    EXPECT_EQ((vector<string>{"tmp abc", "tmp def"}), ci.child_scope_ids());
    EXPECT_EQ(99, ci.version());
    EXPECT_EQ((set<string>{"music", "video"}), ci.keywords());
    EXPECT_TRUE(ci.is_aggregator());

    // Check public assignment operator
    auto tmp = ScopeMetadataImpl::create(move(ti));
    m = tmp;
    EXPECT_EQ("tmp scope_id", m.scope_id());
    EXPECT_EQ("tmp identity", m.proxy()->identity());
    EXPECT_EQ("tmp endpoint", m.proxy()->endpoint());
    EXPECT_EQ("tmp display_name", m.display_name());
    EXPECT_EQ("tmp description", m.description());
    EXPECT_EQ("tmp author", m.author());
    EXPECT_EQ("tmp art", m.art());
    EXPECT_EQ("tmp icon", m.icon());
    EXPECT_EQ("tmp search_hint", m.search_hint());
    EXPECT_EQ("tmp hot_key", m.hot_key());
    EXPECT_EQ("/foo", m.scope_directory());
    EXPECT_EQ("bar", m.appearance_attributes()["foo"].get_string());
    EXPECT_EQ(ScopeMetadata::ResultsTtlType::Small, m.results_ttl_type());
    EXPECT_EQ(99, m.version());
    EXPECT_TRUE(m.invisible());
    EXPECT_EQ(tmp_va, m.settings_definitions());
    EXPECT_TRUE(m.location_data_needed());
    EXPECT_EQ((vector<string>{"tmp abc", "tmp def"}), m.child_scope_ids());
    EXPECT_EQ((set<string>{"music", "video"}), m.keywords());
    EXPECT_TRUE(m.is_aggregator());

    // Self-assignment
    tmp = tmp;
    EXPECT_EQ("tmp scope_id", tmp.scope_id());
    EXPECT_EQ("tmp identity", tmp.proxy()->identity());
    EXPECT_EQ("tmp endpoint", tmp.proxy()->endpoint());
    EXPECT_EQ("tmp display_name", tmp.display_name());
    EXPECT_EQ("tmp description", tmp.description());
    EXPECT_EQ("tmp author", tmp.author());
    EXPECT_EQ("tmp art", tmp.art());
    EXPECT_EQ("tmp icon", tmp.icon());
    EXPECT_EQ("tmp search_hint", tmp.search_hint());
    EXPECT_EQ("tmp hot_key", tmp.hot_key());
    EXPECT_EQ("bar", tmp.appearance_attributes()["foo"].get_string());
    EXPECT_EQ("/foo", tmp.scope_directory());
    EXPECT_EQ(ScopeMetadata::ResultsTtlType::Small, tmp.results_ttl_type());
    EXPECT_EQ(99, tmp.version());
    EXPECT_TRUE(tmp.invisible());
    EXPECT_EQ(tmp_va, tmp.settings_definitions());
    EXPECT_TRUE(tmp.location_data_needed());
    EXPECT_EQ((vector<string>{"tmp abc", "tmp def"}), tmp.child_scope_ids());
    EXPECT_EQ((set<string>{"music", "video"}), tmp.keywords());
    EXPECT_TRUE(tmp.is_aggregator());

    // Copy constructor
    ScopeMetadata tmp2(tmp);
    EXPECT_EQ("tmp scope_id", tmp2.scope_id());
    EXPECT_EQ("tmp identity", tmp2.proxy()->identity());
    EXPECT_EQ("tmp endpoint", tmp2.proxy()->endpoint());
    EXPECT_EQ("tmp display_name", tmp2.display_name());
    EXPECT_EQ("tmp description", tmp2.description());
    EXPECT_EQ("tmp author", tmp2.author());
    EXPECT_EQ("tmp art", tmp2.art());
    EXPECT_EQ("tmp icon", tmp2.icon());
    EXPECT_EQ("tmp search_hint", tmp2.search_hint());
    EXPECT_EQ("tmp hot_key", tmp2.hot_key());
    EXPECT_EQ("/foo", tmp2.scope_directory());
    EXPECT_EQ("bar", tmp2.appearance_attributes()["foo"].get_string());
    EXPECT_EQ(ScopeMetadata::ResultsTtlType::Small, tmp2.results_ttl_type());
    EXPECT_EQ(99, tmp2.version());
    EXPECT_TRUE(tmp2.invisible());
    EXPECT_EQ(tmp_va, tmp2.settings_definitions());
    EXPECT_TRUE(tmp2.location_data_needed());
    EXPECT_EQ((vector<string>{"tmp abc", "tmp def"}), tmp2.child_scope_ids());
    EXPECT_EQ((set<string>{"music", "video"}), tmp2.keywords());
    EXPECT_TRUE(tmp2.is_aggregator());
}

TEST(ScopeMetadataImpl, serialize)
{
    auto rt = RuntimeImpl::create("testscope", runtime_ini);
    ZmqMiddleware mw("testscope", rt.get(), zmq_ini);

    unique_ptr<ScopeMetadataImpl> mi(new ScopeMetadataImpl(&mw));
    mi->set_scope_id("scope_id");
    auto mw_proxy = mw.create_scope_proxy("identity", "endpoint");
    mi->set_proxy(ScopeImpl::create(mw_proxy, "scope_id"));
    mi->set_display_name("display_name");
    mi->set_description("description");
    mi->set_author("author");
    mi->set_art("art");
    mi->set_icon("icon");
    mi->set_search_hint("search_hint");
    mi->set_hot_key("hot_key");
    mi->set_scope_directory("/foo");
    mi->set_invisible(false);
    VariantMap attrs;
    attrs["foo"] = "bar";
    mi->set_appearance_attributes(attrs);
    mi->set_results_ttl_type(ScopeMetadata::ResultsTtlType::Large);
    mi->set_version(1);
    VariantArray va;
    va.push_back(Variant("hello"));
    va.push_back(Variant("world"));
    mi->set_settings_definitions(va);
    mi->set_location_data_needed(false);
    mi->set_child_scope_ids({"com.foo.bar", "com.foo.baz"});
    mi->set_keywords({"news", "games"});
    mi->set_is_aggregator(false);

    // Check that serialize() sets the map values correctly
    auto m = ScopeMetadataImpl::create(move(mi));
    auto var = m.serialize();
    EXPECT_EQ(19u, var.size());
    EXPECT_EQ("scope_id", var["scope_id"].get_string());
    EXPECT_EQ("display_name", var["display_name"].get_string());
    EXPECT_EQ("description", var["description"].get_string());
    EXPECT_EQ("author", var["author"].get_string());
    EXPECT_EQ("art", var["art"].get_string());
    EXPECT_EQ("icon", var["icon"].get_string());
    EXPECT_EQ("search_hint", var["search_hint"].get_string());
    EXPECT_EQ("hot_key", var["hot_key"].get_string());
    EXPECT_EQ("/foo", var["scope_dir"].get_string());
    EXPECT_FALSE(var["invisible"].get_bool());
    EXPECT_EQ("bar", var["appearance_attributes"].get_dict()["foo"].get_string());
    EXPECT_EQ(ScopeMetadata::ResultsTtlType::Large,
              static_cast<ScopeMetadata::ResultsTtlType>(var["results_ttl_type"].get_int()));
    EXPECT_EQ(1, var["version"].get_int());
    EXPECT_FALSE(var["invisible"].get_bool());
    EXPECT_EQ(va, var["settings_definitions"].get_array());
    EXPECT_FALSE(var["location_data_needed"].get_bool());
    EXPECT_EQ(2u, var["child_scopes"].get_array().size());
    EXPECT_EQ("com.foo.bar", var["child_scopes"].get_array()[0].get_string());
    EXPECT_EQ("com.foo.baz", var["child_scopes"].get_array()[1].get_string());
    EXPECT_EQ(2u, var["keywords"].get_array().size());
    EXPECT_EQ("games", var["keywords"].get_array()[0].get_string());
    EXPECT_EQ("news", var["keywords"].get_array()[1].get_string());
    EXPECT_FALSE(var["is_aggregator"].get_bool());

    // Make another instance from the VariantMap and check its fields
    ScopeMetadataImpl c(var, &mw);
    EXPECT_EQ("scope_id", c.scope_id());
    EXPECT_EQ("identity", c.proxy()->identity());
    EXPECT_EQ("endpoint", c.proxy()->endpoint());
    EXPECT_EQ("display_name", c.display_name());
    EXPECT_EQ("description", c.description());
    EXPECT_EQ("author", c.author());
    EXPECT_EQ("art", c.art());
    EXPECT_EQ("icon", c.icon());
    EXPECT_EQ("search_hint", c.search_hint());
    EXPECT_EQ("hot_key", c.hot_key());
    EXPECT_EQ("/foo", c.scope_directory());
    EXPECT_FALSE(c.invisible());
    EXPECT_EQ("bar", c.appearance_attributes()["foo"].get_string());
    EXPECT_EQ(ScopeMetadata::ResultsTtlType::Large, c.results_ttl_type());
    EXPECT_EQ(1, c.version());
    EXPECT_FALSE(c.invisible());
    EXPECT_EQ(va, c.settings_definitions());
    EXPECT_FALSE(c.location_data_needed());
    EXPECT_EQ(2u, c.child_scope_ids().size());
    EXPECT_EQ("com.foo.bar", c.child_scope_ids()[0]);
    EXPECT_EQ("com.foo.baz", c.child_scope_ids()[1]);
    EXPECT_EQ(2u, c.keywords().size());
    EXPECT_NE(c.keywords().end(), c.keywords().find("news"));
    EXPECT_NE(c.keywords().end(), c.keywords().find("games"));
    EXPECT_FALSE(c.is_aggregator());
}

TEST(ScopeMetadataImpl, serialize_exceptions)
{
    auto rt = RuntimeImpl::create("testscope", runtime_ini);
    ZmqMiddleware mw("testscope", rt.get(), zmq_ini);

    ScopeMetadataImpl mi(&mw);
    try
    {
        mi.serialize();
        FAIL();
    }
    catch (InvalidArgumentException const&e)
    {
        EXPECT_STREQ("unity::InvalidArgumentException: ScopeMetadata: required attribute 'scope_id' is empty",
                     e.what());
    }

    try
    {
        mi.set_scope_id("scope_id");
        mi.serialize();
        FAIL();
    }
    catch (InvalidArgumentException const&e)
    {
        EXPECT_STREQ("unity::InvalidArgumentException: ScopeMetadataImpl::serialize(): required attribute 'proxy' is null",
                      e.what());
    }

    try
    {
        auto mw_proxy = mw.create_scope_proxy("identity", "endpoint");
        mi.set_proxy(ScopeImpl::create(mw_proxy, "scope_id"));
        mi.serialize();
        FAIL();
    }
    catch (InvalidArgumentException const&e)
    {
        EXPECT_STREQ("unity::InvalidArgumentException: ScopeMetadata: required attribute 'display_name' is empty",
                      e.what());
    }

    try
    {
        mi.set_display_name("display_name");
        mi.serialize();
        FAIL();
    }
    catch (InvalidArgumentException const&e)
    {
        EXPECT_STREQ("unity::InvalidArgumentException: ScopeMetadata: required attribute 'description' is empty",
                     e.what());
    }

    try
    {
        mi.set_description("description");
        mi.serialize();
        FAIL();
    }
    catch (InvalidArgumentException const&e)
    {
        EXPECT_STREQ("unity::InvalidArgumentException: ScopeMetadata: required attribute 'author' is empty",
                     e.what());
    }
}

TEST(ScopeMetadataImpl, deserialize_exceptions)
{
    auto rt = RuntimeImpl::create("testscope", runtime_ini);
    ZmqMiddleware mw("testscope", rt.get(), zmq_ini);

    VariantMap m;
    try
    {
        ScopeMetadataImpl mi(m, &mw);
        mi.deserialize(m);
        FAIL();
    }
    catch (InvalidArgumentException const&e)
    {
        EXPECT_STREQ("unity::InvalidArgumentException: ScopeMetadata::deserialize(): required attribute "
                     "'scope_id' is missing",
                     e.what());
    }

    m["scope_id"] = "scope_id";
    try
    {
        ScopeMetadataImpl mi(m, &mw);
        mi.deserialize(m);
        FAIL();
    }
    catch (InvalidArgumentException const&e)
    {
        EXPECT_STREQ("unity::InvalidArgumentException: ScopeMetadata::deserialize(): required attribute "
                     "'proxy' is missing",
                     e.what());
    }

    VariantMap proxy;
    m["proxy"] = proxy;
    try
    {
        ScopeMetadataImpl mi(m, &mw);
        mi.deserialize(m);
        FAIL();
    }
    catch (InvalidArgumentException const&e)
    {
        EXPECT_STREQ("unity::InvalidArgumentException: ScopeMetadataImpl::deserialize(): missing 'proxy.identity'",
                     e.what());
    }

    proxy["identity"] = "identity";
    m["proxy"] = proxy;
    try
    {
        ScopeMetadataImpl mi(m, &mw);
        mi.deserialize(m);
        FAIL();
    }
    catch (InvalidArgumentException const&e)
    {
        EXPECT_STREQ("unity::InvalidArgumentException: ScopeMetadataImpl::deserialize(): missing 'proxy.endpoint'",
                     e.what());
    }

    proxy["endpoint"] = "endpoint";
    m["proxy"] = proxy;
    try
    {
        ScopeMetadataImpl mi(m, &mw);
        mi.deserialize(m);
        FAIL();
    }
    catch (InvalidArgumentException const&e)
    {
        EXPECT_STREQ("unity::InvalidArgumentException: ScopeMetadata::deserialize(): required attribute "
                     "'display_name' is missing",
                     e.what());
    }

    m["display_name"] = "display_name";
    try
    {
        ScopeMetadataImpl mi(m, &mw);
        mi.deserialize(m);
        FAIL();
    }
    catch (InvalidArgumentException const&e)
    {
        EXPECT_STREQ("unity::InvalidArgumentException: ScopeMetadata::deserialize(): required attribute "
                     "'description' is missing",
                     e.what());
    }

    m["description"] = "description";
    try
    {
        ScopeMetadataImpl mi(m, &mw);
        mi.deserialize(m);
        FAIL();
    }
    catch (InvalidArgumentException const&e)
    {
        EXPECT_STREQ("unity::InvalidArgumentException: ScopeMetadata::deserialize(): required attribute "
                     "'author' is missing",
                     e.what());
    }

    m["author"] = "author";
    m["results_ttl_type"] = -1;
    try
    {
        ScopeMetadataImpl mi(m, &mw);
        mi.deserialize(m);
        FAIL();
    }
    catch (InvalidArgumentException const&e)
    {
        EXPECT_STREQ("unity::InvalidArgumentException: ScopeMetadata::deserialize(): "
                     "invalid attribute 'results_ttl_type' with value -1",
                     e.what());
    }

    m["results_ttl_type"] = 0;
    m["version"] = -1;
    try
    {
        ScopeMetadataImpl mi(m, &mw);
        mi.deserialize(m);
        FAIL();
    }
    catch (InvalidArgumentException const&e)
    {
        EXPECT_STREQ("unity::InvalidArgumentException: ScopeMetadataImpl::deserialize(): "
                     "invalid attribute 'version' with value -1",
                     e.what());
    }
    m["version"] = 0;

    // Optional attributes
    m["art"] = "art";
    m["icon"] = "icon";
    m["search_hint"] = "search_hint";
    m["hot_key"] = "hot_key";
    m["scope_dir"] = "scope_dir";
    m["invisible"] = true;
    VariantMap appearance;
    appearance["a1"] = "a1";
    m["appearance_attributes"] = appearance;
    m["results_ttl_type"] = 0;
    m["location_data_needed"] = true;
    m["is_aggregator"] = true;

    ScopeMetadataImpl mi(m, &mw);
    mi.deserialize(m);
    EXPECT_EQ("scope_id", mi.scope_id());
    EXPECT_EQ("identity", mi.proxy()->identity());
    EXPECT_EQ("endpoint", mi.proxy()->endpoint());
    EXPECT_EQ("display_name", mi.display_name());
    EXPECT_EQ("description", mi.description());
    EXPECT_EQ("author", mi.author());
    EXPECT_EQ("art", mi.art());
    EXPECT_EQ("icon", mi.icon());
    EXPECT_EQ("search_hint", mi.search_hint());
    EXPECT_EQ("hot_key", mi.hot_key());
    EXPECT_EQ("scope_dir", mi.scope_directory());
    EXPECT_TRUE(mi.invisible());
    EXPECT_EQ(appearance, mi.appearance_attributes());
    EXPECT_EQ(ScopeMetadata::ResultsTtlType::None, mi.results_ttl_type());
    EXPECT_TRUE(mi.location_data_needed());
    EXPECT_TRUE(mi.is_aggregator());
}

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

#include <unity/scopes/internal/ScopeImpl.h>
#include <unity/scopes/internal/zmq_middleware/ZmqMiddleware.h>
#include <unity/scopes/ScopeExceptions.h>
#include <unity/UnityExceptions.h>

#include <boost/regex.hpp>  // Use Boost implementation until http://gcc.gnu.org/bugzilla/show_bug.cgi?id=53631 is fixed.
#include <gtest/gtest.h>
#include <scope-api-testconfig.h>

using namespace std;
using namespace unity;
using namespace unity::scopes;
using namespace unity::scopes::internal;
using namespace unity::scopes::internal::zmq_middleware;

TEST(ScopeMetadataImpl, basic)
{
    ZmqMiddleware mw("testscope", TEST_BUILD_ROOT "/gtest/scopes/internal/ScopeMetadataImpl/Zmq.ini",
                     (RuntimeImpl*)0x1);

    unique_ptr<ScopeMetadataImpl> mi(new ScopeMetadataImpl(&mw));
    mi->set_scope_id("scope_name");
    auto mw_proxy = mw.create_scope_proxy("identity", "endpoint");
    mi->set_proxy(ScopeImpl::create(mw_proxy, mw.runtime(), "scope_name"));
    mi->set_display_name("display_name");
    mi->set_description("description");
    mi->set_author("author");

    // Keep a copy for tests below
    unique_ptr<ScopeMetadataImpl> mi2(new ScopeMetadataImpl(*mi));

    // Create the public instance and check that the values match
    auto m = ScopeMetadataImpl::create(move(mi));
    EXPECT_EQ("scope_name", m.scope_id());
    EXPECT_EQ("identity", m.proxy()->identity());
    EXPECT_EQ("endpoint", m.proxy()->endpoint());
    EXPECT_EQ("display_name", m.display_name());
    EXPECT_EQ("description", m.description());
    EXPECT_EQ("author", m.author());

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

    // when "invisible" is not set, false is returned
    EXPECT_FALSE(m.invisible());

    // Check that the copy has the same values as the original
    EXPECT_EQ("scope_name", mi2->scope_id());
    EXPECT_EQ("identity", mi2->proxy()->identity());
    EXPECT_EQ("endpoint", mi2->proxy()->endpoint());
    EXPECT_EQ("display_name", mi2->display_name());
    EXPECT_EQ("description", mi2->description());
    EXPECT_EQ("author", mi2->author());

    // Set optional fields on copy.
    mi2->set_art("art");
    mi2->set_icon("icon");
    mi2->set_search_hint("search_hint");
    mi2->set_hot_key("hot_key");
    mi2->set_invisible(true);

    // Make another copy, so we get coverage on the entire copy constructor
    unique_ptr<ScopeMetadataImpl> mi3(new ScopeMetadataImpl(*mi2));

    // Check that optional fields are set correctly
    m = ScopeMetadataImpl::create(move(mi2));
    EXPECT_EQ("search_hint", m.search_hint());
    EXPECT_EQ("hot_key", m.hot_key());
    EXPECT_TRUE(m.invisible());

    // Make another value
    unique_ptr<ScopeMetadataImpl> ti(new ScopeMetadataImpl(&mw));
    ti->set_scope_id("tmp scope_name");
    mw_proxy = mw.create_scope_proxy("tmp identity", "tmp endpoint");
    ti->set_proxy(ScopeImpl::create(mw_proxy, mw.runtime(), "tmp scope_name"));
    ti->set_display_name("tmp display_name");
    ti->set_description("tmp description");
    ti->set_author("tmp author");
    ti->set_art("tmp art");
    ti->set_icon("tmp icon");
    ti->set_search_hint("tmp search_hint");
    ti->set_hot_key("tmp hot_key");
    ti->set_invisible(true);

    // Check impl assignment operator
    ScopeMetadataImpl ci(&mw);
    ci = *ti;
    EXPECT_EQ("tmp scope_name", ci.scope_id());
    EXPECT_EQ("tmp identity", ci.proxy()->identity());
    EXPECT_EQ("tmp endpoint", ci.proxy()->endpoint());
    EXPECT_EQ("tmp display_name", ci.display_name());
    EXPECT_EQ("tmp description", ci.description());
    EXPECT_EQ("tmp author", ci.author());
    EXPECT_EQ("tmp art", ci.art());
    EXPECT_EQ("tmp icon", ci.icon());
    EXPECT_EQ("tmp search_hint", ci.search_hint());
    EXPECT_EQ("tmp hot_key", ci.hot_key());
    EXPECT_TRUE(ci.invisible());

    // Check public assignment operator
    auto tmp = ScopeMetadataImpl::create(move(ti));
    m = tmp;
    EXPECT_EQ("tmp scope_name", m.scope_id());
    EXPECT_EQ("tmp identity", m.proxy()->identity());
    EXPECT_EQ("tmp endpoint", m.proxy()->endpoint());
    EXPECT_EQ("tmp display_name", m.display_name());
    EXPECT_EQ("tmp description", m.description());
    EXPECT_EQ("tmp author", m.author());
    EXPECT_EQ("tmp art", m.art());
    EXPECT_EQ("tmp icon", m.icon());
    EXPECT_EQ("tmp search_hint", m.search_hint());
    EXPECT_EQ("tmp hot_key", m.hot_key());
    EXPECT_TRUE(m.invisible());

    // Self-assignment
    tmp = tmp;
    EXPECT_EQ("tmp scope_name", tmp.scope_id());
    EXPECT_EQ("tmp identity", tmp.proxy()->identity());
    EXPECT_EQ("tmp endpoint", tmp.proxy()->endpoint());
    EXPECT_EQ("tmp display_name", tmp.display_name());
    EXPECT_EQ("tmp description", tmp.description());
    EXPECT_EQ("tmp author", tmp.author());
    EXPECT_EQ("tmp art", tmp.art());
    EXPECT_EQ("tmp icon", tmp.icon());
    EXPECT_EQ("tmp search_hint", tmp.search_hint());
    EXPECT_EQ("tmp hot_key", tmp.hot_key());
    EXPECT_TRUE(tmp.invisible());

    // Copy constructor
    ScopeMetadata tmp2(tmp);
    EXPECT_EQ("tmp scope_name", tmp2.scope_id());
    EXPECT_EQ("tmp identity", tmp2.proxy()->identity());
    EXPECT_EQ("tmp endpoint", tmp2.proxy()->endpoint());
    EXPECT_EQ("tmp display_name", tmp2.display_name());
    EXPECT_EQ("tmp description", tmp2.description());
    EXPECT_EQ("tmp author", tmp2.author());
    EXPECT_EQ("tmp art", tmp2.art());
    EXPECT_EQ("tmp icon", tmp2.icon());
    EXPECT_EQ("tmp search_hint", tmp2.search_hint());
    EXPECT_EQ("tmp hot_key", tmp2.hot_key());
    EXPECT_TRUE(tmp2.invisible());
}

TEST(ScopeMetadataImpl, serialize)
{
    ZmqMiddleware mw("testscope", TEST_BUILD_ROOT "/gtest/scopes/internal/ScopeMetadataImpl/Zmq.ini",
                     (RuntimeImpl*)0x1);

    unique_ptr<ScopeMetadataImpl> mi(new ScopeMetadataImpl(&mw));
    mi->set_scope_id("scope_name");
    auto mw_proxy = mw.create_scope_proxy("identity", "endpoint");
    mi->set_proxy(ScopeImpl::create(mw_proxy, mw.runtime(), "scope_name"));
    mi->set_display_name("display_name");
    mi->set_description("description");
    mi->set_author("author");
    mi->set_art("art");
    mi->set_icon("icon");
    mi->set_search_hint("search_hint");
    mi->set_hot_key("hot_key");
    mi->set_invisible(false);

    // Check that serialize() sets the map values correctly
    auto m = ScopeMetadataImpl::create(move(mi));
    auto var = m.serialize();
    EXPECT_EQ(10u, var.size());
    EXPECT_EQ("scope_name", var["scope_id"].get_string());
    EXPECT_EQ("display_name", var["display_name"].get_string());
    EXPECT_EQ("description", var["description"].get_string());
    EXPECT_EQ("author", var["author"].get_string());
    EXPECT_EQ("art", var["art"].get_string());
    EXPECT_EQ("icon", var["icon"].get_string());
    EXPECT_EQ("search_hint", var["search_hint"].get_string());
    EXPECT_EQ("hot_key", var["hot_key"].get_string());
    EXPECT_FALSE(var["invisible"].get_bool());

    // Make another instance from the VariantMap and check its fields
    ScopeMetadataImpl c(var, &mw);
    EXPECT_EQ("scope_name", c.scope_id());
    EXPECT_EQ("identity", c.proxy()->identity());
    EXPECT_EQ("endpoint", c.proxy()->endpoint());
    EXPECT_EQ("display_name", c.display_name());
    EXPECT_EQ("description", c.description());
    EXPECT_EQ("author", c.author());
    EXPECT_EQ("art", c.art());
    EXPECT_EQ("icon", c.icon());
    EXPECT_EQ("search_hint", c.search_hint());
    EXPECT_EQ("hot_key", c.hot_key());
    EXPECT_FALSE(c.invisible());
}

TEST(ScopeMetadataImpl, serialize_exceptions)
{
    ZmqMiddleware mw("testscope", TEST_BUILD_ROOT "/gtest/scopes/internal/ScopeMetadataImpl/Zmq.ini",
                     (RuntimeImpl*)0x1);

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
        mi.set_scope_id("scope_name");
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
        mi.set_proxy(ScopeImpl::create(mw_proxy, mw.runtime(), "scope_name"));
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
    ZmqMiddleware mw("testscope", TEST_BUILD_ROOT "/gtest/scopes/internal/ScopeMetadataImpl/Zmq.ini",
                     (RuntimeImpl*)0x1);

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

    m["scope_id"] = "scope_name";
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

    // Optional attributes
    m["art"] = "art";
    m["icon"] = "icon";
    m["search_hint"] = "search_hint";
    m["hot_key"] = "hot_key";

    ScopeMetadataImpl mi(m, &mw);
    mi.deserialize(m);
    EXPECT_EQ("scope_name", mi.scope_id());
    EXPECT_EQ("identity", mi.proxy()->identity());
    EXPECT_EQ("endpoint", mi.proxy()->endpoint());
    EXPECT_EQ("display_name", mi.display_name());
    EXPECT_EQ("description", mi.description());
    EXPECT_EQ("author", mi.author());
    EXPECT_EQ("art", mi.art());
    EXPECT_EQ("icon", mi.icon());
    EXPECT_EQ("search_hint", mi.search_hint());
    EXPECT_EQ("hot_key", mi.hot_key());
}

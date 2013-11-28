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

#include <scopes/internal/ScopeMetadataImpl.h>

#include <scopes/internal/ScopeImpl.h>
#include <scopes/internal/zmq_middleware/ZmqMiddleware.h>
#include <scopes/ScopeExceptions.h>
#include <unity/UnityExceptions.h>

#include <boost/regex.hpp>  // Use Boost implementation until http://gcc.gnu.org/bugzilla/show_bug.cgi?id=53631 is fixed.
#include <gtest/gtest.h>
#include <scope-api-testconfig.h>

using namespace std;
using namespace unity;
using namespace unity::api::scopes;
using namespace unity::api::scopes::internal;
using namespace unity::api::scopes::internal::zmq_middleware;

TEST(ScopeMetadataImpl, basic)
{
    ZmqMiddleware mw("testscope", TEST_BUILD_ROOT "/gtest/unity/api/scopes/internal/ScopeMetadataImpl/Zmq.ini",
                     (RuntimeImpl*)0x1);

    unique_ptr<ScopeMetadataImpl> mi(new ScopeMetadataImpl(&mw));
    mi->set_scope_name("scope_name");
    mi->set_art("art");
    auto mw_proxy = mw.create_scope_proxy("identity", "endpoint");
    mi->set_proxy(ScopeImpl::create(mw_proxy, mw.runtime()));
    mi->set_localized_name("localized_name");
    mi->set_description("description");

    // Keep a copy for tests below
    unique_ptr<ScopeMetadataImpl> mi2(new ScopeMetadataImpl(*mi));

    // Create the public instance and check that the values match
    auto m = ScopeMetadataImpl::create(move(mi));
    EXPECT_EQ("scope_name", m.scope_name());
    EXPECT_EQ("art", m.art());
    EXPECT_EQ("identity", m.proxy()->identity());
    EXPECT_EQ("endpoint", m.proxy()->endpoint());
    EXPECT_EQ("localized_name", m.localized_name());
    EXPECT_EQ("description", m.description());

    // Check that optional fields that are not set throw
    try
    {
        m.search_hint();
        FAIL();
    }
    catch (NotFoundException const& e)
    {
        EXPECT_EQ("unity::api::scopes::NotFoundException: attribute not set (name = search_hint)", e.to_string());
    }

    try
    {
        m.hot_key();
        FAIL();
    }
    catch (NotFoundException const& e)
    {
        EXPECT_EQ("unity::api::scopes::NotFoundException: attribute not set (name = hot_key)", e.to_string());
    }

    // Check that the copy has the same values as the original
    EXPECT_EQ("scope_name", mi2->scope_name());
    EXPECT_EQ("art", mi2->art());
    EXPECT_EQ("identity", mi2->proxy()->identity());
    EXPECT_EQ("endpoint", mi2->proxy()->endpoint());
    EXPECT_EQ("localized_name", mi2->localized_name());
    EXPECT_EQ("description", mi2->description());

    // Set optional fields on copy.
    mi2->set_search_hint("search_hint");
    mi2->set_hot_key("hot_key");

    // Make another copy, so we get coverage on the entire copy constructor
    unique_ptr<ScopeMetadataImpl> mi3(new ScopeMetadataImpl(*mi2));

    // Check that optional fields are set correctly
    m = ScopeMetadataImpl::create(move(mi2));
    EXPECT_EQ("search_hint", m.search_hint());
    EXPECT_EQ("hot_key", m.hot_key());

    // Make another value
    unique_ptr<ScopeMetadataImpl> ti(new ScopeMetadataImpl(&mw));
    ti->set_scope_name("tmp scope_name");
    ti->set_art("tmp art");
    mw_proxy = mw.create_scope_proxy("tmp identity", "tmp endpoint");
    ti->set_proxy(ScopeImpl::create(mw_proxy, mw.runtime()));
    ti->set_localized_name("tmp localized_name");
    ti->set_description("tmp description");
    ti->set_search_hint("tmp search_hint");
    ti->set_hot_key("tmp hot_key");

    // Check impl assignment operator
    ScopeMetadataImpl ci(&mw);
    ci = *ti;
    EXPECT_EQ("tmp scope_name", ci.scope_name());
    EXPECT_EQ("tmp art", ci.art());
    EXPECT_EQ("tmp identity", ci.proxy()->identity());
    EXPECT_EQ("tmp endpoint", ci.proxy()->endpoint());
    EXPECT_EQ("tmp localized_name", ci.localized_name());
    EXPECT_EQ("tmp description", ci.description());
    EXPECT_EQ("tmp search_hint", ci.search_hint());
    EXPECT_EQ("tmp hot_key", ci.hot_key());

    // Check public assignment operator
    auto tmp = ScopeMetadataImpl::create(move(ti));
    m = tmp;
    EXPECT_EQ("tmp scope_name", m.scope_name());
    EXPECT_EQ("tmp art", m.art());
    EXPECT_EQ("tmp identity", m.proxy()->identity());
    EXPECT_EQ("tmp endpoint", m.proxy()->endpoint());
    EXPECT_EQ("tmp localized_name", m.localized_name());
    EXPECT_EQ("tmp description", m.description());
    EXPECT_EQ("tmp search_hint", m.search_hint());
    EXPECT_EQ("tmp hot_key", m.hot_key());

    // Self-assignment
    tmp = tmp;
    EXPECT_EQ("tmp scope_name", tmp.scope_name());
    EXPECT_EQ("tmp art", tmp.art());
    EXPECT_EQ("tmp identity", tmp.proxy()->identity());
    EXPECT_EQ("tmp endpoint", tmp.proxy()->endpoint());
    EXPECT_EQ("tmp localized_name", tmp.localized_name());
    EXPECT_EQ("tmp description", tmp.description());
    EXPECT_EQ("tmp search_hint", tmp.search_hint());
    EXPECT_EQ("tmp hot_key", tmp.hot_key());

    // Copy constructor
    ScopeMetadata tmp2(tmp);
    EXPECT_EQ("tmp scope_name", tmp2.scope_name());
    EXPECT_EQ("tmp art", tmp2.art());
    EXPECT_EQ("tmp identity", tmp2.proxy()->identity());
    EXPECT_EQ("tmp endpoint", tmp2.proxy()->endpoint());
    EXPECT_EQ("tmp localized_name", tmp2.localized_name());
    EXPECT_EQ("tmp description", tmp2.description());
    EXPECT_EQ("tmp search_hint", tmp2.search_hint());
    EXPECT_EQ("tmp hot_key", tmp2.hot_key());
}

TEST(ScopeMetadataImpl, serialize)
{
    ZmqMiddleware mw("testscope", TEST_BUILD_ROOT "/gtest/unity/api/scopes/internal/ScopeMetadataImpl/Zmq.ini",
                     (RuntimeImpl*)0x1);

    unique_ptr<ScopeMetadataImpl> mi(new ScopeMetadataImpl(&mw));
    mi->set_scope_name("scope_name");
    mi->set_art("art");
    auto mw_proxy = mw.create_scope_proxy("identity", "endpoint");
    mi->set_proxy(ScopeImpl::create(mw_proxy, mw.runtime()));
    mi->set_localized_name("localized_name");
    mi->set_description("description");
    mi->set_search_hint("search_hint");
    mi->set_hot_key("hot_key");

    // Check that serialize() sets the map values correctly
    auto m = ScopeMetadataImpl::create(move(mi));
    auto var = m.serialize();
    EXPECT_EQ(7, var.size());
    EXPECT_EQ("scope_name", var["scope_name"].get_string());
    EXPECT_EQ("art", var["art"].get_string());
    EXPECT_EQ("localized_name", var["localized_name"].get_string());
    EXPECT_EQ("description", var["description"].get_string());
    EXPECT_EQ("search_hint", var["search_hint"].get_string());
    EXPECT_EQ("hot_key", var["hot_key"].get_string());

    // Make another instance from the VariantMap and check its fields
    ScopeMetadataImpl c(var, &mw);
    EXPECT_EQ("scope_name", c.scope_name());
    EXPECT_EQ("art", c.art());
    EXPECT_EQ("identity", c.proxy()->identity());
    EXPECT_EQ("endpoint", c.proxy()->endpoint());
    EXPECT_EQ("localized_name", c.localized_name());
    EXPECT_EQ("description", c.description());
    EXPECT_EQ("search_hint", c.search_hint());
    EXPECT_EQ("hot_key", c.hot_key());
}

TEST(ScopeMetadataImpl, serialize_exceptions)
{
    ZmqMiddleware mw("testscope", TEST_BUILD_ROOT "/gtest/unity/api/scopes/internal/ScopeMetadataImpl/Zmq.ini",
                     (RuntimeImpl*)0x1);

    ScopeMetadataImpl mi(&mw);
    try
    {
        mi.serialize();
    }
    catch (InvalidArgumentException const&e)
    {
        EXPECT_EQ("unity::InvalidArgumentException: ScopeMetadata: required attribute 'scope_name' is empty",
                   e.to_string());
    }

    try
    {
        mi.set_scope_name("scope_name");
        mi.serialize();
    }
    catch (InvalidArgumentException const&e)
    {
        EXPECT_EQ("unity::InvalidArgumentException: ScopeMetadata: required attribute 'art' is empty",
                   e.to_string());
    }

    try
    {
        mi.set_art("art");
        mi.serialize();
    }
    catch (InvalidArgumentException const&e)
    {
        EXPECT_EQ("unity::InvalidArgumentException: ScopeMetadataImpl::serialize(): required attribute 'proxy' is null",
                   e.to_string());
    }

    try
    {
        auto mw_proxy = mw.create_scope_proxy("identity", "endpoint");
        mi.set_proxy(ScopeImpl::create(mw_proxy, mw.runtime()));
        mi.serialize();
    }
    catch (InvalidArgumentException const&e)
    {
        EXPECT_EQ("unity::InvalidArgumentException: ScopeMetadata: required attribute 'localized_name' is empty",
                   e.to_string());
    }

    try
    {
        mi.set_localized_name("localized_name");
        mi.serialize();
    }
    catch (InvalidArgumentException const&e)
    {
        EXPECT_EQ("unity::InvalidArgumentException: ScopeMetadata: required attribute 'description' is empty",
                   e.to_string());
    }
}

TEST(ScopeMetadataImpl, deserialize_exceptions)
{
    ZmqMiddleware mw("testscope", TEST_BUILD_ROOT "/gtest/unity/api/scopes/internal/ScopeMetadataImpl/Zmq.ini",
                     (RuntimeImpl*)0x1);

    VariantMap m;
    try
    {
        ScopeMetadataImpl mi(m, &mw);
        mi.deserialize(m);
    }
    catch (InvalidArgumentException const&e)
    {
        EXPECT_EQ("unity::InvalidArgumentException: ScopeMetadata::deserialize(): required attribute "
                  "'scope_name' is missing",
                   e.to_string());
    }

    m["scope_name"] = "scope_name";
    try
    {
        ScopeMetadataImpl mi(m, &mw);
        mi.deserialize(m);
    }
    catch (InvalidArgumentException const&e)
    {
        EXPECT_EQ("unity::InvalidArgumentException: ScopeMetadata::deserialize(): required attribute "
                  "'art' is missing",
                   e.to_string());
    }

    m["art"] = "art";
    try
    {
        ScopeMetadataImpl mi(m, &mw);
        mi.deserialize(m);
    }
    catch (InvalidArgumentException const&e)
    {
        EXPECT_EQ("unity::InvalidArgumentException: ScopeMetadata::deserialize(): required attribute "
                  "'proxy' is missing",
                   e.to_string());
    }

    VariantMap proxy;
    m["proxy"] = proxy;
    try
    {
        ScopeMetadataImpl mi(m, &mw);
        mi.deserialize(m);
    }
    catch (InvalidArgumentException const&e)
    {
        EXPECT_EQ("unity::InvalidArgumentException: ScopeMetadataImpl::deserialize(): missing 'proxy.identity'",
                   e.to_string());
    }

    proxy["identity"] = "identity";
    m["proxy"] = proxy;
    try
    {
        ScopeMetadataImpl mi(m, &mw);
        mi.deserialize(m);
    }
    catch (InvalidArgumentException const&e)
    {
        EXPECT_EQ("unity::InvalidArgumentException: ScopeMetadataImpl::deserialize(): missing 'proxy.endpoint'",
                   e.to_string());
    }

    proxy["endpoint"] = "endpoint";
    m["proxy"] = proxy;
    try
    {
        ScopeMetadataImpl mi(m, &mw);
        mi.deserialize(m);
    }
    catch (InvalidArgumentException const&e)
    {
        EXPECT_EQ("unity::InvalidArgumentException: ScopeMetadata::deserialize(): required attribute "
                  "'localized_name' is missing",
                   e.to_string());
    }

    m["localized_name"] = "localized_name";
    try
    {
        ScopeMetadataImpl mi(m, &mw);
        mi.deserialize(m);
    }
    catch (InvalidArgumentException const&e)
    {
        EXPECT_EQ("unity::InvalidArgumentException: ScopeMetadata::deserialize(): required attribute "
                  "'description' is missing",
                   e.to_string());
    }
    m["description"] = "description";

    // Optional attributes
    m["search_hint"] = "search_hint";
    m["hot_key"] = "hot_key";

    ScopeMetadataImpl mi(m, &mw);
    mi.deserialize(m);
    EXPECT_EQ("scope_name", mi.scope_name());
    EXPECT_EQ("art", mi.art());
    EXPECT_EQ("identity", mi.proxy()->identity());
    EXPECT_EQ("endpoint", mi.proxy()->endpoint());
    EXPECT_EQ("localized_name", mi.localized_name());
    EXPECT_EQ("description", mi.description());
    EXPECT_EQ("search_hint", mi.search_hint());
    EXPECT_EQ("hot_key", mi.hot_key());
}

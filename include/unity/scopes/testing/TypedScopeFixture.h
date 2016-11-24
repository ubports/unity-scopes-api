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
 * Authored by: Thomas Voß <thomas.voss@canonical.com>
 */

#pragma once

#include <unity/scopes/Version.h>

#include <unity/scopes/testing/MockRegistry.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <gtest/gtest.h>
#pragma GCC diagnostic pop

#include <memory>

namespace unity
{

namespace scopes
{

class ScopeBase;

namespace testing
{

/// @cond

template<typename Scope>
struct ScopeTraits
{
    inline static const char* name()
    {
        return "unknown";
    }

    inline static std::shared_ptr<Scope> construct()
    {
        return std::make_shared<Scope>();
    }
};

class TypedScopeFixtureHelper
{
    static void set_scope_directory(std::shared_ptr<ScopeBase> const& scope, std::string const& path);
    static void set_cache_directory(std::shared_ptr<ScopeBase> const& scope, std::string const& path);
    static void set_app_directory(std::shared_ptr<ScopeBase> const& scope, std::string const& path);
    static void set_tmp_directory(std::shared_ptr<ScopeBase> const& scope, std::string const& path);
    static void set_registry(std::shared_ptr<ScopeBase> const& scope, RegistryProxy const& r);

    template<typename Scope>
    friend class TypedScopeFixture;
};

/// @endcond

/**
 \brief Fixture for testing scope testing.

 This fixture template provides convienience SetUp() and TearDown() functions, as well as
 a number of setters that ease the creation of scope tests.
 */
template<typename Scope>
class TypedScopeFixture : public ::testing::Test
{
public:
/// @cond
    TypedScopeFixture()
        : registry_proxy(&registry, [](unity::scopes::Registry*) {})
        , scope(ScopeTraits<Scope>::construct())
    {
        TypedScopeFixtureHelper::set_registry(scope, registry_proxy);
        TypedScopeFixtureHelper::set_scope_directory(scope, "/tmp");
        TypedScopeFixtureHelper::set_app_directory(scope, "/tmp");
    }

    void SetUp()
    {
        ASSERT_NO_THROW(scope->start(ScopeTraits<Scope>::name()));
        ASSERT_NO_THROW(scope->run());
    }

    void set_scope_directory(std::string const& path)
    {
        TypedScopeFixtureHelper::set_scope_directory(scope, path);
    }

    void set_cache_directory(std::string const& path)
    {
        TypedScopeFixtureHelper::set_cache_directory(scope, path);
    }

    void set_app_directory(std::string const& path)
    {
        TypedScopeFixtureHelper::set_app_directory(scope, path);
    }

    void set_tmp_directory(std::string const& path)
    {
        TypedScopeFixtureHelper::set_tmp_directory(scope, path);
    }

    static void set_registry(std::shared_ptr<ScopeBase> const& scope, RegistryProxy const& r)
    {
        TypedScopeFixtureHelper::set_registry(scope, r);
    }

    void TearDown()
    {
        EXPECT_NO_THROW(scope->stop());
    }

protected:
    unity::scopes::testing::MockRegistry registry;
    unity::scopes::RegistryProxy registry_proxy;
    std::shared_ptr<Scope> scope;
/// @endcond
};


} // namespace testing

} // namespace scopes

} // namespace unity

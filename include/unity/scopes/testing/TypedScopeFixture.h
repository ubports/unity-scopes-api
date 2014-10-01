/*
 * Copyright (C) 2013 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the Lesser GNU General Public License version 3 as
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

#ifndef UNITY_SCOPES_TESTING_TYPED_SCOPE_FIXTURE_H
#define UNITY_SCOPES_TESTING_TYPED_SCOPE_FIXTURE_H

#include <unity/scopes/Version.h>

#include <unity/scopes/testing/MockRegistry.h>

#include <gtest/gtest.h>

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

struct TypedScopeFixtureHelper
{
private:
    static void set_registry(std::shared_ptr<ScopeBase> const& scope, RegistryProxy const& r);

    static void set_scope_directory(std::shared_ptr<ScopeBase> const& scope, std::string const& path);

    template<typename Scope>
    friend class TypedScopeFixture;
};

template<typename Scope>
class TypedScopeFixture : public ::testing::Test
{
public:
    TypedScopeFixture()
        : registry_proxy(&registry, [](unity::scopes::Registry*) {})
        , scope(ScopeTraits<Scope>::construct())
    {
        TypedScopeFixtureHelper::set_registry(scope, registry_proxy);
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

    void TearDown()
    {
        EXPECT_NO_THROW(scope->stop());
    }

protected:
    unity::scopes::testing::MockRegistry registry;
    unity::scopes::RegistryProxy registry_proxy;
    std::shared_ptr<Scope> scope;
};

/// @endcond

} // namespace testing

} // namespace scopes

} // namespace unity
#endif

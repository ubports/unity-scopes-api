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

#include <unity/scopes/Registry.h>
#include <unity/scopes/testing/MockObject.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <gmock/gmock.h>
#pragma GCC diagnostic pop

namespace unity
{

namespace scopes
{

namespace testing
{

/// @cond

class MockRegistry : public Registry, public virtual MockObject
{
public:
    MockRegistry() = default;

    MOCK_METHOD1(get_metadata, ScopeMetadata(std::string const&));
    MOCK_METHOD0(list, MetadataMap());
    MOCK_METHOD1(list_if, MetadataMap(std::function<bool(ScopeMetadata const&)>));
    MOCK_METHOD1(is_scope_running, bool(std::string const&));
    core::ScopedConnection set_scope_state_callback(std::string const&, std::function<void(bool is_running)>) override
    {
        return core::Signal<>().connect([]{});
    }
    core::ScopedConnection set_list_update_callback(std::function<void()>) override
    {
        return core::Signal<>().connect([]{});
    }
};

/// @endcond

} // namespace testing

} // namespace scopes

} // namespace unity

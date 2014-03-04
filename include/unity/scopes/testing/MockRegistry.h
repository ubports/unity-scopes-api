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

#ifndef UNITY_SCOPES_TESTING_MOCK_REGISTRY_H
#define UNITY_SCOPES_TESTING_MOCK_REGISTRY_H

#include <unity/scopes/Registry.h>

#include <gmock/gmock.h>

namespace unity
{

namespace scopes
{

namespace testing
{

/// @cond

class MockRegistry : public Registry
{
public:
    MockRegistry() = default;

    MOCK_CONST_METHOD1(get_metadata, ScopeMetadata(std::string const&));
    MOCK_CONST_METHOD0(list, MetadataMap());
    MOCK_CONST_METHOD1(list_if, MetadataMap(std::function<bool(ScopeMetadata const&)>));
};

/// @endcond

} // namespace testing

} // namespace scopes

} // namespace unity

#endif

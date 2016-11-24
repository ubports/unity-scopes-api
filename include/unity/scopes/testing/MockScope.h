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
 * Authored by: James Henstridge <james.henstridge@canonical.com>
 */

#pragma once

#include <unity/scopes/Scope.h>
#include <unity/scopes/SearchMetadata.h>
#include <unity/scopes/ActionMetadata.h>

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

class MockScope : public unity::scopes::Scope, public unity::scopes::testing::MockObject
{
public:
    MockScope(std::string const& endpoint = "",
              std::string const& identity = "")
        : MockObject(endpoint, identity) {}

    MOCK_METHOD3(search, QueryCtrlProxy(std::string const&,
                                        SearchMetadata const&,
                                        SearchListenerBase::SPtr const&));
    MOCK_METHOD4(search, QueryCtrlProxy(std::string const&,
                                        FilterState const&,
                                        SearchMetadata const&,
                                        SearchListenerBase::SPtr const&));
    MOCK_METHOD5(search, QueryCtrlProxy(std::string const&,
                                        std::string const&,
                                        FilterState const&,
                                        SearchMetadata const&,
                                        SearchListenerBase::SPtr const&));
    MOCK_METHOD3(activate, QueryCtrlProxy(unity::scopes::Result const&,
                                          ActionMetadata const&,
                                          ActivationListenerBase::SPtr const&));
    MOCK_METHOD5(perform_action, QueryCtrlProxy(unity::scopes::Result const&,
                                                ActionMetadata const&,
                                                std::string const&,
                                                std::string const&,
                                                ActivationListenerBase::SPtr const&));
    MOCK_METHOD3(preview, QueryCtrlProxy(unity::scopes::Result const&,
                                         ActionMetadata const&,
                                         PreviewListenerBase::SPtr const&));

    MOCK_METHOD0(child_scopes, ChildScopeList());

    MOCK_METHOD6(search, QueryCtrlProxy(std::string const&,
                                        std::string const&,
                                        FilterState const&,
                                        Variant const&,
                                        SearchMetadata const&,
                                        SearchListenerBase::SPtr const&));

    MOCK_METHOD1(set_child_scopes, bool(ChildScopeList const&));
    MOCK_METHOD4(activate_result_action, QueryCtrlProxy(unity::scopes::Result const&,
                                                        ActionMetadata const&,
                                                        std::string const&,
                                                        ActivationListenerBase::SPtr const&));
};

/// @endcond

} // namespace testing

} // namespace scopes

} // namespace unity

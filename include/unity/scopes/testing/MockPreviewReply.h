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

#ifndef UNITY_SCOPES_TESTING_MOCK_PREVIEW_REPLY_H
#define UNITY_SCOPES_TESTING_MOCK_PREVIEW_REPLY_H

#include <unity/scopes/PreviewReply.h>

#include <gmock/gmock.h>

namespace unity
{

namespace scopes
{

namespace testing
{

/// @cond

class MockPreviewReply : public unity::scopes::PreviewReply
{
public:
    MockPreviewReply() = default;

    // From Reply
    MOCK_CONST_METHOD0(finished, void());
    MOCK_CONST_METHOD1(error, void(std::exception_ptr));

    // From SearchReply
    MOCK_CONST_METHOD1(register_layout, bool(ColumnLayoutList const&));
    MOCK_CONST_METHOD1(push, bool(PreviewWidgetList const&));
    MOCK_CONST_METHOD2(push, bool(std::string const&, Variant const&));
};

/// @endcond

} // namespace testing

} // namespace scopes

} // namespace unity

#endif

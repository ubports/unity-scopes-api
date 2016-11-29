/*
 * Copyright (C) 2015 Canonical Ltd
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
 * Authored by: Xavi Garcia <xavi.garcia.mena@canonical.com>
 */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <gmock/gmock.h>
#include <gmock/gmock-actions.h>
#pragma GCC diagnostic pop

#include "QMockScope.h"
#include "QScopeBaseAPIMock.h"

#include <unity/scopes/qt/internal/QScopeBaseAPIImpl.h>

#include <unity/scopes/internal/ResultImpl.h>
#include <unity/scopes/ActionMetadata.h>

using namespace testing;

using namespace unity::scopes::qt;
using namespace unity::scopes::qt::internal;

TEST(TestSetup, bindings)
{
    QScopeMock* scope = new QScopeMock;

    // construct the QSearchQueryBaseAPIMock
    QScopeBaseAPIMock api_scope([scope]{ return scope; });

    // verify that the event method is called for start event
    EXPECT_CALL(*scope, start(_)).Times(Exactly(1));
    api_scope.start("test_scope");

    QThread* qt_thread = api_scope.get_qt_app_thread();
    scope->set_qt_thread(qt_thread);

    auto check_thread = [qt_thread]() -> void
    {
        EXPECT_EQ(qt_thread, QThread::currentThread());
    };

    unity::scopes::internal::ResultImpl resultImpl;
    resultImpl.set_uri("test_uri");

    unity::scopes::Result result = unity::scopes::internal::ResultImpl::create_result(resultImpl.serialize());

    unity::scopes::CannedQuery query("scopeA", "query", "department");
    unity::scopes::ActionMetadata action_metadata("en", "phone");

    // verify that the event method is called for stop event and
    // that the thread is the Qt thread
    EXPECT_CALL(*scope, stop()).Times(Exactly(1)).WillOnce(Invoke(check_thread));
    api_scope.stop();
}

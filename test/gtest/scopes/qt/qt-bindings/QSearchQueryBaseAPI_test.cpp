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

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "TestSetup.h"
#include "FakeScope.h"
#include "QEventTypeMatcher.h"

#include <unity/scopes/qt/QSearchQueryBaseAPI.h>

#include <chrono>

using namespace testing;

using namespace unity::scopes::qt;

class QSearchQueryBaseAPIMock : public QSearchQueryBaseAPI
{
public:
    QSearchQueryBaseAPIMock(std::shared_ptr<QCoreApplication> qtapp,
                            QScopeBase& qtscope,
                            unity::scopes::CannedQuery const& query,
                            unity::scopes::SearchMetadata const& metadata,
                            QObject* parent = 0)
        : QSearchQueryBaseAPI(qtapp, qtscope, query, metadata, parent)
    {
    }

    virtual ~QSearchQueryBaseAPIMock() = default;

    void callCancel()
    {
        cancelled();
    }

    void callRun()
    {
        unity::scopes::SearchReplyProxy reply;
        // call with a null reply, as we are not going to use it
        // in the mock call
        run(reply);
    }

    // mock event method
    MOCK_METHOD1(event, bool(QEvent*));
};

// For type numbers refer to QSearchQueryBaseAPI.cpp
Matcher<QEvent*> CheckCancelledEventType(void* thread_id)
{
    return MakeMatcher(new QEventTypeMatcher(1002, thread_id));
}

// For type numbers refer to QSearchQueryBaseAPI.cpp
Matcher<QEvent*> CheckInitializeEventType(void* thread_id)
{
    return MakeMatcher(new QEventTypeMatcher(1000, thread_id));
}

// For type numbers refer to QSearchQueryBaseAPI.cpp
Matcher<QEvent*> CheckRunEventType(void* thread_id)
{
    return MakeMatcher(new QEventTypeMatcher(1001, thread_id));
}

TEST_F(TestSetup, bindings)
{
    QScope scope;
    unity::scopes::CannedQuery query("scopeA", "query", "department");
    unity::scopes::SearchMetadata metadata("en", "phone");

    // construct the QSearchQueryBaseAPIMock
    QSearchQueryBaseAPIMock api_query(qtapp_, scope, query, metadata);

    // give some time to process the events posted in the constructor
    std::chrono::milliseconds dura(500);
    std::this_thread::sleep_for(dura);

    // verify that the event method is called for cancel event and
    // from the same thread id (The Qt thread)
    EXPECT_CALL(api_query, event(CheckCancelledEventType(thread_id_))).Times(Exactly(1));
    api_query.callCancel();

    // verify that the event method is called for cancel event and
    // from the same thread id (The Qt thread)
    EXPECT_CALL(api_query, event(CheckRunEventType(thread_id_))).Times(Exactly(1));
    api_query.callRun();

    std::this_thread::sleep_for(dura);
}

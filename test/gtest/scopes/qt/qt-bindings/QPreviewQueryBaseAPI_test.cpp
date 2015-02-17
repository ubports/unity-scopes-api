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

#include "BasicEventsChecker.h"
#include "TestSetup.h"
#include "FakeScope.h"
#include "QEventTypeMatcher.h"

#include <unity/scopes/qt/QPreviewQueryBaseAPI.h>

#include <unity/scopes/internal/ResultImpl.h>
#include <unity/scopes/ActionMetadata.h>

#include <chrono>

using namespace testing;

using namespace unity::scopes::qt;

// For type numbers refer to QSearchQueryBaseAPI.cpp
const int INITIALIZE_EVENT = 1000;
const int RUN_EVENT = 1001;
const int CANCEL_EVENT = 1002;

// in milliseconds
// 5 seconds should be more than enough
// If we don't receive an event in 5 seconds something
// is going very wrong.
const int DEFAULT_TIME_TO_WAIT = 5000;


class QPreviewQueryBaseAPIMock : public QPreviewQueryBaseAPI,
                                 public BasicEventsChecker
{
public:
    QPreviewQueryBaseAPIMock(std::shared_ptr<QCoreApplication> qtapp,
                            QScopeBase& qtscope,
                            unity::scopes::Result const& result,
                            unity::scopes::ActionMetadata const& metadata,
                            QObject *parent=0)
                            : QPreviewQueryBaseAPI(qtapp,qtscope,result, metadata,parent),
                              BasicEventsChecker()
    {
    }

    virtual ~QPreviewQueryBaseAPIMock() = default;

    void callCancel()
    {
        cancelled();
    }

    void callRun()
    {
        unity::scopes::PreviewReplyProxy reply;
        // call with a null reply, as we are not going to use it
        // in the mock call
        run(reply);
    }

    // override the method to implement the checks
    bool event(QEvent* e) override
    {
        return this->check_event(e);
    }
};

// For type numbers refer to QSearchQueryBaseAPI.cpp
Matcher<QEvent *> CheckCancelledEventType(void *thread_id)
{
    return MakeMatcher(new QEventTypeMatcher(CANCEL_EVENT, thread_id));
}

// For type numbers refer to QSearchQueryBaseAPI.cpp
Matcher<QEvent *> CheckInitializeEventType(void *thread_id)
{
    return MakeMatcher(new QEventTypeMatcher(INITIALIZE_EVENT, thread_id));
}

// For type numbers refer to QSearchQueryBaseAPI.cpp
Matcher<QEvent *> CheckRunEventType(void *thread_id)
{
    return MakeMatcher(new QEventTypeMatcher(RUN_EVENT, thread_id));
}

void verifyEvent(QPreviewQueryBaseAPIMock &api_query, int event, int timeout)
{
    if(!api_query.nbEventCalls(event))
    {
        EXPECT_TRUE(api_query.waitForEvent(event,timeout));
        EXPECT_EQ(1, api_query.nbEventCalls(event));
    }
    else
    {
        EXPECT_EQ(1, api_query.nbEventCalls(event));
    }
}

TEST_F(TestSetup, bindings)
{
    QScope scope;

    unity::scopes::internal::ResultImpl *resultImpl = new unity::scopes::internal::ResultImpl();
    resultImpl->set_uri("test_uri");

    unity::scopes::Result result = unity::scopes::internal::ResultImpl::create_result(resultImpl->serialize());

    unity::scopes::CannedQuery query("scopeA", "query", "department");
    unity::scopes::ActionMetadata metadata("en", "phone");

    //construct the QSearchQueryBaseAPIMock
    QPreviewQueryBaseAPIMock api_query(qtapp_, scope, result, metadata);
    api_query.setThreadId(thread_id_);

    verifyEvent(api_query, INITIALIZE_EVENT, DEFAULT_TIME_TO_WAIT);

    // verify run event.
    EXPECT_EQ(0, api_query.nbEventCalls(RUN_EVENT));
    api_query.callRun();
    verifyEvent(api_query, RUN_EVENT, DEFAULT_TIME_TO_WAIT);

    // verify cancel
    EXPECT_EQ(0, api_query.nbEventCalls(CANCEL_EVENT));
    api_query.callCancel();
    verifyEvent(api_query, CANCEL_EVENT, DEFAULT_TIME_TO_WAIT);

    // final verification
    EXPECT_EQ(1, api_query.nbEventCalls(INITIALIZE_EVENT));
    EXPECT_EQ(1, api_query.nbEventCalls(RUN_EVENT));
    EXPECT_EQ(1, api_query.nbEventCalls(CANCEL_EVENT));

    // Now we can exit without any sleep as we know that no other event
    // is going to be sent
}

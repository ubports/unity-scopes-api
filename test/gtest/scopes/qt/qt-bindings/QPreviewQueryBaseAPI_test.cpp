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

#include <unity/scopes/qt/QPreviewQueryBaseAPI.h>

#include <unity/scopes/internal/ResultImpl.h>
#include <unity/scopes/ActionMetadata.h>

#include <chrono>

using namespace testing;

using namespace unity::scopes::qt;

class QPreviewQueryBaseAPIMock : public QPreviewQueryBaseAPI
{
public:
    QPreviewQueryBaseAPIMock(std::shared_ptr<QCoreApplication> qtapp,
                             QScopeBase& qtscope,
                             unity::scopes::Result const& result,
                             unity::scopes::ActionMetadata const& metadata,
                             QObject* parent = nullptr)
        : QPreviewQueryBaseAPI(qtapp, qtscope, result, metadata, parent)
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

    // mock event method
    MOCK_METHOD1(event, bool(QEvent*));
};

// For type numbers refer to QSearchQueryBaseAPI.cpp
Matcher<QEvent*> CheckCancelledEventType(void* thread_id)
{
    return MakeMatcher(new QEventTypeMatcher(1002, thread_id));
}

// For type numbers refer to QSearchQueryBaseAPI.cpp
Matcher<QEvent*> CheckRunEventType(void* thread_id)
{
    return MakeMatcher(new QEventTypeMatcher(1001, thread_id));
}

TEST_F(TestSetup, bindings)
{
    QScope scope;

    unity::scopes::internal::ResultImpl* resultImpl = new unity::scopes::internal::ResultImpl();
    resultImpl->set_uri("test_uri");

    unity::scopes::Result result = unity::scopes::internal::ResultImpl::create_result(resultImpl->serialize());

    unity::scopes::CannedQuery query("scopeA", "query", "department");
    unity::scopes::ActionMetadata metadata("en", "phone");

    // construct the QSearchQueryBaseAPIMock
    QPreviewQueryBaseAPIMock api_query(qtapp_, scope, result, metadata);

    // give some time to process the events posted in the constructor
    std::chrono::milliseconds dura(1000);
    std::this_thread::sleep_for(dura);

    // verify that the event method is called for cancel event and
    // from the same thread id (The Qt thread)
    EXPECT_CALL(api_query, event(CheckCancelledEventType(thread_id_))).Times(Exactly(1));
    api_query.callCancel();

    // verify that the event method is called for run event and
    // from the same thread id (The Qt thread)
    EXPECT_CALL(api_query, event(CheckRunEventType(thread_id_))).Times(Exactly(1));
    api_query.callRun();

    std::this_thread::sleep_for(dura);
}

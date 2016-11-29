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
#pragma GCC diagnostic pop

#include "BasicEventsChecker.h"
#include "TestSetup.h"
#include "FakeScope.h"
#include "QEventTypeMatcher.h"

#include <unity/scopes/qt/QSearchQueryBaseAPI.h>

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

namespace unity
{

namespace scopes
{

namespace qt
{

namespace tests
{

class QSearchQueryBaseAPIMock : public QSearchQueryBaseAPI, public BasicEventsChecker
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

    void call_cancel()
    {
        cancelled();
    }

    void call_run()
    {
        unity::scopes::SearchReplyProxy reply;
        // call with a null reply, as we are not going to use it
        // in the mock call
        run(reply);
    }

    void call_init()
    {
        init();
    }

    // override the method to implement the checks
    bool event(QEvent* e) override
    {
        return this->check_event(e);
    }
};

}  // namespace tests

}  // namespace qt

}  // namespace scopes

}  // namespace unity

using namespace unity::scopes::qt::tests;

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

void verify_event(QSearchQueryBaseAPIMock& api_query, int event, int timeout)
{
    if(!api_query.nb_event_calls(event))
    {
        api_query.wait_for_event(event, timeout);
        EXPECT_EQ(1, api_query.nb_event_calls(event));
    }
}

TEST_F(TestSetup, bindings)
{
    QScope scope;
    unity::scopes::CannedQuery query("scopeA", "query", "department");
    unity::scopes::SearchMetadata metadata("en", "phone");

    // construct the QSearchQueryBaseAPIMock
    QSearchQueryBaseAPIMock api_query(qtapp_, scope, query, metadata);
    api_query.set_thread_id(thread_id_);

    EXPECT_EQ(0, api_query.nb_event_calls(INITIALIZE_EVENT));
    api_query.call_init();
    verify_event(api_query, INITIALIZE_EVENT, DEFAULT_TIME_TO_WAIT);

    // verify run event.
    EXPECT_EQ(0, api_query.nb_event_calls(RUN_EVENT));
    api_query.call_run();
    verify_event(api_query, RUN_EVENT, DEFAULT_TIME_TO_WAIT);

    // verify cancel
    EXPECT_EQ(0, api_query.nb_event_calls(CANCEL_EVENT));
    api_query.call_cancel();
    verify_event(api_query, CANCEL_EVENT, DEFAULT_TIME_TO_WAIT);

    // final verification
    EXPECT_EQ(1, api_query.nb_event_calls(INITIALIZE_EVENT));
    EXPECT_EQ(1, api_query.nb_event_calls(RUN_EVENT));
    EXPECT_EQ(1, api_query.nb_event_calls(CANCEL_EVENT));

    // Now we can exit without any sleep as we know that no other event
    // is going to be sent
}

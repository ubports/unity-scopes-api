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

#include <unity/scopes/qt/QSearchQueryBase.h>
#include <unity/scopes/qt/QSearchQueryBaseAPI.h>
#include <unity/scopes/qt/QSearchReplyProxy.h>
#include <unity/scopes/qt/QScopeBase.h>

#include <unity/scopes/SearchMetadata.h>

#include <QtCore/QCoreApplication>

#include <thread>
#include <cassert>

using namespace unity::scopes::qt;
using namespace unity::scopes;

// Qt user events start at QEvent::User, which is 1000...
enum EventType
{
    Initialize = QEvent::User,
    Run,
    Cancelled
};

/// @cond
class InitializeEvent : public QEvent
{
public:
    InitializeEvent()
        : QEvent(static_cast<QEvent::Type>(Initialize))
    {
    }
};

class CancelledEvent : public QEvent
{
public:
    CancelledEvent()
        : QEvent(static_cast<QEvent::Type>(Cancelled))
    {
    }
};

class RunEvent : public QEvent
{
public:
    RunEvent(unity::scopes::SearchReplyProxy proxy)
        : QEvent(static_cast<QEvent::Type>(Run))
        , proxy_(proxy)
    {
    }

    unity::scopes::SearchReplyProxy proxy_;
};

QSearchQueryBaseAPI::QSearchQueryBaseAPI(std::shared_ptr<QCoreApplication> app,
                                         QScopeBase& qtscope,
                                         CannedQuery const& query,
                                         SearchMetadata const& metadata,
                                         QObject* parent)
    : QObject(parent)
    , SearchQueryBase(query, metadata)
    , qtapp_(app)
    , qtscope_(qtscope)
{
    // move the object to the Qt thread
    this->moveToThread(app->thread());

    // Post event to initialize the object in the Qt thread
    app->postEvent(this, new InitializeEvent());
}

QSearchQueryBaseAPI::~QSearchQueryBaseAPI()
{
}

bool QSearchQueryBaseAPI::event(QEvent* e)
{
    RunEvent* run_event = nullptr;
    EventType type = static_cast<EventType>(e->type());

    QSearchReplyProxy qt_search_reply;
    switch (type)
    {
        case Initialize:
            // initialize the query object
            qtquery_ = qtscope_.search(SearchQueryBase::query(), SearchQueryBase::search_metadata());
            qtquery_->init(this);
            break;
        case Run:
            assert(qtquery_);
            run_event = dynamic_cast<RunEvent*>(e);
            assert(run_event);
            // execute the run method that must be implemented by the user
            qt_search_reply.reset(new QSearchReply(run_event->proxy_));
            qtquery_->run(qt_search_reply);
            break;
        case Cancelled:
            assert(qtquery_);
            qtquery_->cancelled();
            break;
        default:
            break;
    }
    return true;
}

void QSearchQueryBaseAPI::run(SearchReplyProxy const& reply)
{
    // we are called fron a non Qt thread, so now we push a new Qt event to the
    // Qt event loop
    qtapp_->postEvent(this, new RunEvent(reply));
}

void QSearchQueryBaseAPI::cancelled()
{
    // we are called fron a non Qt thread, so now we push a new Qt event to the
    // Qt event loop
    qtapp_->postEvent(this, new CancelledEvent());
}
/// @endcond

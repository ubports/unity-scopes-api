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

#include <unity/scopes/qt/QPreviewQueryBase.h>
#include <unity/scopes/qt/QPreviewQueryBaseAPI.h>
#include <unity/scopes/qt/QScopeBase.h>
#include <unity/scopes/qt/QResult.h>
#include <unity/scopes/qt/QActionMetadata.h>

#include <unity/scopes/qt/internal/QResultImpl.h>
#include <unity/scopes/qt/internal/QActionMetadataImpl.h>

#include <unity/scopes/ActionMetadata.h>

#include <QtCore/QCoreApplication>

#include <thread>
#include <cassert>

using namespace unity::scopes::qt;
using namespace unity::scopes;

/// @cond
// Qt user events start at QEvent::User, which is 1000...
enum EventType
{
    Initialize = QEvent::User,
    Run,
    Cancelled
};

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
    RunEvent(unity::scopes::PreviewReplyProxy proxy)
        : QEvent(static_cast<QEvent::Type>(Run))
        , proxy_(proxy)
    {
    }

    unity::scopes::PreviewReplyProxy proxy_;
};

QPreviewQueryBaseAPI::QPreviewQueryBaseAPI(std::shared_ptr<QCoreApplication> qtapp,
                                           QScopeBase& qtscope,
                                           unity::scopes::Result const& result,
                                           unity::scopes::ActionMetadata const& metadata,
                                           QObject *parent)
    : QObject(parent)
    , PreviewQueryBase(result, metadata)
    , qtapp_(qtapp)
    , qtscope_(qtscope)
{
    // move the object to the Qt thread
    this->moveToThread(qtapp_->thread());

    // Post event to initialize the object in the Qt thread
    qtapp_->postEvent(this, new InitializeEvent());
}

QPreviewQueryBaseAPI::~QPreviewQueryBaseAPI()
{
}

bool QPreviewQueryBaseAPI::event(QEvent* e)
{
    RunEvent* run_event = nullptr;
    EventType type = static_cast<EventType>(e->type());

    QPreviewReplyProxy qt_preview_reply;
    switch (type)
    {
        case Initialize:
            // initialize the query object
            qtquery_ = qtscope_.preview(
                QResult(new internal::QResultImpl(PreviewQueryBase::result())),
                QActionMetadata(new internal::QActionMetadataImpl(PreviewQueryBase::action_metadata())));
            qtquery_->init(this);
            break;
        case Run:
            assert(qtquery_);
            run_event = dynamic_cast<RunEvent*>(e);
            assert(run_event);
            // execute the run method that must be implemented by the user
            qt_preview_reply.reset(new QPreviewReply(run_event->proxy_));
            qtquery_->run(qt_preview_reply);
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

void QPreviewQueryBaseAPI::run(unity::scopes::PreviewReplyProxy const& reply)
{
    // we are called fron a non Qt thread, so now we push a new Qt event to the
    // Qt event loop
    qtapp_->postEvent(this, new RunEvent(reply));
}

void QPreviewQueryBaseAPI::cancelled()
{
    // we are called fron a non Qt thread, so now we push a new Qt event to the
    // Qt event loop
    qtapp_->postEvent(this, new CancelledEvent());
}
/// @endcond

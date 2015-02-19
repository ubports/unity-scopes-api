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

#include <unity/scopes/qt/internal/QScopeBaseAPIImpl.h>
#include <unity/scopes/qt/QScopeBase.h>
#include <unity/scopes/qt/QSearchQueryBaseAPI.h>
#include <unity/scopes/qt/QPreviewQueryBaseAPI.h>

#include <QtCore/QCoreApplication>
#include <QtCore/QThread>

#include <cassert>
#include <chrono>

using namespace std;
using namespace unity::scopes::qt::internal;
namespace sc = unity::scopes;

namespace unity
{

namespace scopes
{

namespace qt
{

namespace internal
{

// Qt user events start at QEvent::User, which is 1000...
enum EventType
{
    Start = QEvent::User,
    Stop
};

class StartEvent : public QEvent
{
public:
    StartEvent(QString const& scope_id)
        : QEvent(static_cast<QEvent::Type>(Start))
        , scope_id_(scope_id)
    {
    }
    QString scope_id_;
};

class StopEvent : public QEvent
{
public:
    StopEvent()
        : QEvent(static_cast<QEvent::Type>(Stop))
    {
    }
};

}  // namespace internal

}  // namespace qt

}  // namespace scopes

}  // namespace unity

QScopeBaseAPIImpl::QScopeBaseAPIImpl(QScopeBase& qtscope, QObject* parent)
    : QObject(parent)
    , qtapp_ready_(false)
    , qtscope_impl_(&qtscope)
{
}

QScopeBaseAPIImpl::QScopeBaseAPIImpl(FactoryFunc const& creator, QObject* parent)
    : QObject(parent)
    , qtapp_ready_(false)
    , qtscope_impl_(nullptr)
    , qtscope_creator_(creator)
{
}

QScopeBaseAPIImpl::~QScopeBaseAPIImpl()
{
    qtthread_->join();
}

bool QScopeBaseAPIImpl::event(QEvent* e)
{
    StartEvent* start_event = nullptr;
    EventType type = static_cast<EventType>(e->type());

    QSearchReplyProxy qt_search_reply;
    switch (type)
    {
        case Start:
            // create the client's scope in the
            // Qt main thread
            if (!qtscope_impl_)
            {
                qtscope_impl_ = qtscope_creator_();
                assert(qtscope_impl_);
            }
            // Move the user's scope to the Qt main thread
            qtscope_impl_->moveToThread(qtapp_->thread());

            start_event = dynamic_cast<StartEvent*>(e);
            qtscope_impl_->start(start_event->scope_id_);
            break;
        case Stop:
            qtscope_impl_->stop();

            // exit the QCoreApplication
            qtapp_->quit();
            break;
        default:
            break;
    }
    return true;
}

void QScopeBaseAPIImpl::start(std::string const& scope_id)
{
    // start the QT thread
    // TODO change to make_unique when using C++14
    qtthread_ = std::unique_ptr<std::thread>(new std::thread(&QScopeBaseAPIImpl::startQtThread, this));
    while (!qtapp_ready_)
    {
        std::chrono::milliseconds dura(10);
        std::this_thread::sleep_for(dura);
    }

    // Move this class to the Qt main thread
    this->moveToThread(qtapp_->thread());

    // now we can call start in the client's scope
    // Post event to initialize the object in the Qt thread
    qtapp_->postEvent(this, new StartEvent(scope_id.c_str()));
}

void QScopeBaseAPIImpl::stop()
{
    // Post event to initialize the object in the Qt thread
    qtapp_->postEvent(this, new StopEvent());
}

sc::PreviewQueryBase::UPtr QScopeBaseAPIImpl::preview(const sc::Result& result, const sc::ActionMetadata& metadata)
{
    // Boilerplate construction of Preview
    QPreviewQueryBaseAPI * preview_api = new QPreviewQueryBaseAPI(qtapp_, *qtscope_impl_, result, metadata);
    preview_api->init();
    return sc::PreviewQueryBase::UPtr(preview_api);
}

/**
 * Called each time a new query is requested
 */
sc::SearchQueryBase::UPtr QScopeBaseAPIImpl::search(sc::CannedQuery const& query, sc::SearchMetadata const& metadata)
{
    // Boilerplate construction of Query
    QSearchQueryBaseAPI* query_api = new QSearchQueryBaseAPI(qtapp_, *qtscope_impl_, query, metadata);
    query_api->init();
    return sc::SearchQueryBase::UPtr(query_api);
}

void QScopeBaseAPIImpl::startQtThread()
{
    if (!QCoreApplication::instance())
    {
        int argc = 0;
        char* argv = NULL;
        qtapp_ = std::make_shared<QCoreApplication>(argc, &argv);
        qtapp_ready_ = true;
        qtapp_->exec();
        // delete QtCoreApplication in the same thread it was created
        qtapp_.reset();
    }
}

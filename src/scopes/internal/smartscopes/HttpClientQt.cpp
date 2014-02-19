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
 * Authored by: Marcus Tomlinson <marcus.tomlinson@canonical.com>
 */

#include <unity/scopes/internal/smartscopes/HttpClientQt.h>
#include <unity/scopes/internal/smartscopes/HttpClientQtThread.h>
#include <unity/UnityExceptions.h>

#include <QCoreApplication>
#include <QEventLoop>
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>

#include <iostream>

using namespace unity::scopes::internal::smartscopes;

//-- HttpClientQt

HttpClientQt::HttpClientQt(uint max_sessions, uint no_reply_timeout)
    : session_index_(0)
    , max_sessions_(max_sessions > 0 ? max_sessions : 1)
    , no_reply_timeout_(no_reply_timeout)
    , app_(nullptr)
{
    if (!QCoreApplication::instance())
    {
        int argc = 0;
        app_ = std::unique_ptr<QCoreApplication>(new QCoreApplication(argc, nullptr));
    }
}

HttpClientQt::~HttpClientQt()
{
}

HttpResponseHandle::SPtr HttpClientQt::get(std::string const& request_url, uint port)
{
    std::lock_guard<std::mutex> lock(sessions_mutex_);

    while (sessions_.size() >= max_sessions_)
    {
        auto it = sessions_.begin();
        it->second->wait_for_session();
        sessions_.erase(it);
    }

    // start new session
    auto session = std::make_shared<HttpSession>(request_url, port, no_reply_timeout_);
    sessions_[session_index_] = session;

    return std::make_shared<HttpResponseHandle>(session_index_++, session->get_future());
}

void HttpClientQt::cancel_get(const HttpResponseHandle::SPtr& session_handle)
{
    std::lock_guard<std::mutex> lock(sessions_mutex_);

    // if session_id in map, cancel it
    auto it = sessions_.find(session_handle->session_id());
    if (it != sessions_.end())
    {
        it->second->cancel_session();
        sessions_.erase(it);
    }
}

std::string HttpClientQt::to_percent_encoding(std::string const& string)
{
    return QUrl::toPercentEncoding(string.c_str()).constData();
}

//-- HttpClientQt::HttpSession

HttpClientQt::HttpSession::HttpSession(std::string const& request_url, int port, uint timeout)
    : promise_(nullptr)
    , qt_thread_(nullptr)
{
    promise_ = std::make_shared<std::promise<std::string>>();

    get_thread_ =
        std::thread([this, request_url, port, timeout]()
            {
                QUrl url(request_url.c_str());

                if (port != 0)
                {
                    url.setPort(port);
                }

                {
                    std::lock_guard<std::mutex> lock(qt_thread_mutex_);
                    qt_thread_ = std::unique_ptr<HttpClientQtThread>(new HttpClientQtThread(url, timeout));
                }

                QEventLoop loop;
                QObject::connect(qt_thread_.get(), &HttpClientQtThread::finished, &loop, &QEventLoop::quit);

                qt_thread_->start();
                qt_thread_ready_.set_value();
                loop.exec();

                std::string reply;
                bool success = qt_thread_->get_reply(reply);

                if (!success)
                {
                    unity::ResourceException e(reply);
                    promise_->set_exception(e.self());
                }
                else
                {
                    promise_->set_value(reply);
                }
            });

    qt_thread_ready_.get_future().wait();
}

HttpClientQt::HttpSession::~HttpSession()
{
    cancel_session();
}

std::future<std::string> HttpClientQt::HttpSession::get_future()
{
    return promise_->get_future();
}

void HttpClientQt::HttpSession::cancel_session()
{
    {
        std::lock_guard<std::mutex> lock(qt_thread_mutex_);
        qt_thread_->cancel();
    }

    wait_for_session();
}

void HttpClientQt::HttpSession::wait_for_session()
{
    if (get_thread_.joinable())
    {
        get_thread_.join();
    }
}

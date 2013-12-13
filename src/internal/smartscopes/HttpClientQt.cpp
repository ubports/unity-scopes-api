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

#include <scopes/internal/smartscopes/HttpClientQt.h>
#include <scopes/internal/smartscopes/HttpClientQtThread.h>
#include <unity/UnityExceptions.h>

#include <QCoreApplication>
#include <QEventLoop>
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>

#include <iostream>

using namespace unity::api::scopes::internal::smartscopes;

//-- HttpClientQt

HttpClientQt::HttpClientQt(uint max_sessions)
    : max_sessions_(max_sessions > 0 ? max_sessions : 1),
      app_(nullptr)
{
    if (!QCoreApplication::instance())
    {
        int argc = 0;
        app_ = new QCoreApplication(argc, nullptr);
    }
}

HttpClientQt::~HttpClientQt()
{
    for (auto it = begin(sessions_); it != end(sessions_); it++)
    {
        it->second->cancel_session();
    }

    delete app_;
}

std::future<std::string> HttpClientQt::get(const std::string& request_url, const std::string& session_id, int port)
{
    // if session_id already in map, cancel it
    auto it = sessions_.find(session_id);
    if (it != sessions_.end())
    {
        it->second->cancel_session();
        sessions_.erase(it);
    }
    // if session_id not in map, wait for next available slot
    else
    {
        while (sessions_.size() >= max_sessions_)
        {
            it = sessions_.begin();
            it->second->wait_for_session();
            sessions_.erase(it);
        }
    }

    // start new session
    auto session = std::make_shared <HttpSession> (request_url, port);
    sessions_[session_id] = session;

    return session->get_future();
}

std::string HttpClientQt::to_percent_encoding(const std::string& string)
{
    return QUrl::toPercentEncoding(string.c_str()).constData();
}

//-- HttpClientQt::HttpSession

HttpClientQt::HttpSession::HttpSession(const std::string& request_url, int port)
    : promise_(nullptr),
      get_thread_(nullptr),
      get_qthread_(nullptr)
{
    promise_ = std::make_shared<std::promise<std::string>>();

    get_thread_ = std::unique_ptr <std::thread> (new std::thread([this, request_url, port]()
    {
        QUrl url(request_url.c_str());
        url.setPort(port);
        get_qthread_ = std::unique_ptr <HttpClientQtThread> (new HttpClientQtThread(url));

        QEventLoop loop;
        QObject::connect(get_qthread_.get(), SIGNAL(finished()), &loop, SLOT(quit()));

        get_qthread_->start();
        loop.exec();
        get_qthread_->wait();

        QNetworkReply* reply = get_qthread_->getReply();

        if (!reply)
        {
            // no reply
            unity::ResourceException e("No reply from " + request_url + ":" + std::to_string(port));
            promise_->set_exception(make_exception_ptr(e));
        }
        else if (reply->error() != QNetworkReply::NoError)
        {
            // communication error
            unity::ResourceException e(reply->errorString().toStdString());
            promise_->set_exception(make_exception_ptr(e));
        }
        else
        {
            QString reply_string(reply->readAll());
            promise_->set_value(reply_string.toStdString());
        }
    }));
}

std::future<std::string> HttpClientQt::HttpSession::get_future()
{
    return promise_->get_future();
}

void HttpClientQt::HttpSession::cancel_session()
{
    while (!get_qthread_)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    get_qthread_->cancel();
    wait_for_session();
}

void HttpClientQt::HttpSession::wait_for_session()
{
    get_thread_->join();
}

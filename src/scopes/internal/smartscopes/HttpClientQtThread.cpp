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

#include "unity/scopes/internal/smartscopes/HttpClientQtThread.h"

#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>

HttpClientQtThread::HttpClientQtThread(const QUrl& url, uint timeout)
    : QThread(),
      url_(url),
      timeout_(timeout),
      success_(false)
{
}

HttpClientQtThread::~HttpClientQtThread()
{
    cancel();

    quit();
    wait();
}

bool HttpClientQtThread::get_reply(std::string& reply)
{
    std::lock_guard<std::mutex> lock(reply_mutex_);

    reply = reply_;
    return success_;
}

void HttpClientQtThread::run()
{
    QNetworkAccessManager* manager = new QNetworkAccessManager();

    QNetworkRequest request(url_);

    QNetworkReply* reply = manager->get(request);

    connect(manager, &QNetworkAccessManager::finished, this, &HttpClientQtThread::got_reply);
    connect(this, &HttpClientQtThread::abort, reply, &QNetworkReply::abort);

    QTimer timeout;
    timeout.singleShot(timeout_, this, SLOT(timeout()));
    QThread::exec(); // enter event loop

    delete reply;
    delete manager;
}

void HttpClientQtThread::cancel()
{
    std::lock_guard<std::mutex> lock(reply_mutex_);

    success_ = false;
    reply_ = "Request to " + url_.url().toStdString() + ":" + std::to_string(url_.port()) + " cancelled";

    emit abort();
    emit query_done();
}

void HttpClientQtThread::timeout()
{
    std::lock_guard<std::mutex> lock(reply_mutex_);

    success_ = false;
    reply_ = "Request to " + url_.url().toStdString() + ":" + std::to_string(url_.port()) + " timed out";

    emit abort();
    emit query_done();
}

void HttpClientQtThread::got_reply(QNetworkReply* reply)
{
    std::lock_guard<std::mutex> lock(reply_mutex_);

    if (!reply)
    {
        // no reply
        success_ = false;
        reply_ = "No reply from " + url_.url().toStdString() + ":" + std::to_string(url_.port());
    }
    else if (!reply->isFinished())
    {
        // incomplete reply
        success_ = false;
        reply_ = "Incomplete reply from " + url_.url().toStdString() + ":" + std::to_string(url_.port());
    }
    else if (reply->error() != QNetworkReply::NoError)
    {
        // communication error
        success_ = false;
        reply_ = reply->errorString().toStdString();
    }
    else
    {
        success_ = true;
        reply_ = QString(reply->readAll()).toStdString();
    }

    emit query_done();
}

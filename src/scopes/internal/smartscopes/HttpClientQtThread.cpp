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
      reply_(nullptr),
      manager_(nullptr)
{
}

HttpClientQtThread::~HttpClientQtThread()
{
    cancel();

    reply_->deleteLater();
    manager_->deleteLater();
}

void HttpClientQtThread::run()
{
    {
        std::lock_guard<std::mutex> lock(reply_mutex_);

        manager_ = new QNetworkAccessManager();

        QNetworkRequest request(url_);

        reply_ = manager_->get(request);

        connect(manager_, &QNetworkAccessManager::finished, this, &HttpClientQtThread::queryDone);
        connect(this, &HttpClientQtThread::abort, reply_, &QNetworkReply::abort);
    }

    QTimer timeout;
    timeout.singleShot(timeout_, this, SLOT(cancel()));
    QThread::exec(); // enter event loop
}

QNetworkReply* HttpClientQtThread::getReply()
{
    std::lock_guard<std::mutex> lock(reply_mutex_);
    return reply_;
}

void HttpClientQtThread::queryDone(QNetworkReply*)
{
    quit();
}

void HttpClientQtThread::cancel()
{
    std::lock_guard<std::mutex> lock(reply_mutex_);

    if (reply_)
    {
        emit abort();
    }

    quit();
}

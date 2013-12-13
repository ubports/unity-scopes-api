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

#include "scopes/internal/smartscopes/HttpClientQtThread.h"

#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>

HttpClientQtThread::HttpClientQtThread(const QUrl& url, uint no_reply_timeout, const HttpHeadersList& headers)
    : QThread(),
      url_(url),
      headers_(headers),
      no_reply_timeout_( no_reply_timeout ),
      reply_(nullptr),
      manager_(nullptr)
{
}

HttpClientQtThread::~HttpClientQtThread()
{
    cancel();
}

void HttpClientQtThread::run()
{
    manager_ = new QNetworkAccessManager();

    QNetworkRequest request(url_);
    for (auto it = headers_.begin(); it != headers_.end(); it++)
    {
        request.setRawHeader((*it).first, (*it).second);
    }

    connect(manager_, SIGNAL( finished( QNetworkReply* ) ), this, SLOT( queryDone( QNetworkReply* ) ));
    reply_ = manager_->get(request);

    QTimer timeout;
    timeout.singleShot(no_reply_timeout_, this, SLOT( cancel() ));

    QThread::exec(); // enter event loop
}

QNetworkReply* HttpClientQtThread::getReply() const
{
    return reply_;
}

void HttpClientQtThread::queryDone(QNetworkReply*)
{
    quit();
}

void HttpClientQtThread::cancel()
{
    if (reply_)
    {
        reply_->abort();
        delete reply_;
        reply_ = nullptr;
    }

    if (manager_)
    {
        delete manager_;
        manager_ = nullptr;
    }

    quit();
}

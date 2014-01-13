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

#ifndef UNITY_SCOPES_INTERNAL_SMARTSCOPES_HTTPCLIENTQTTHREAD_H
#define UNITY_SCOPES_INTERNAL_SMARTSCOPES_HTTPCLIENTQTTHREAD_H

#include <unity/util/NonCopyable.h>

#include <QThread>
#include <QUrl>
#include <mutex>

class QNetworkReply;
class QNetworkAccessManager;

class Q_DECL_EXPORT HttpClientQtThread : public QThread
{
    Q_OBJECT

public:
    NONCOPYABLE(HttpClientQtThread);

    HttpClientQtThread(const QUrl& url, uint timeout);
    ~HttpClientQtThread();

    void run();
    QNetworkReply* getReply();

public Q_SLOTS:
    void cancel();
    void queryDone(QNetworkReply*);

Q_SIGNALS:
    void abort();

private:
    QUrl url_;
    uint timeout_;
    QNetworkReply* reply_;
    QNetworkAccessManager* manager_;
    std::mutex reply_mutex_;
};

#endif // UNITY_SCOPES_INTERNAL_SMARTSCOPES_HTTPCLIENTQTTHREAD_H

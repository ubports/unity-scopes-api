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

#ifndef UNITY_API_SCOPES_INTERNAL_SMARTSCOPES_HTTPCLIENTQTTHREAD_H
#define UNITY_API_SCOPES_INTERNAL_SMARTSCOPES_HTTPCLIENTQTTHREAD_H

#include <unity/util/NonCopyable.h>

#include <QThread>
#include <QUrl>

class QNetworkReply;
class QNetworkAccessManager;

typedef QPair<QByteArray, QByteArray> HttpHeader;
typedef QList<HttpHeader> HttpHeadersList;

class Q_DECL_EXPORT HttpClientQtThread : public QThread,
    private unity::util::NonCopyable
{
    Q_OBJECT

public:
    explicit HttpClientQtThread( const QUrl& url, const HttpHeadersList& = HttpHeadersList() );
    ~HttpClientQtThread();

    void run();
    QNetworkReply* getReply() const;

public Q_SLOTS:
    void cancel();
    void queryDone( QNetworkReply* );

private:
    QUrl m_url;
    HttpHeadersList m_headers;
    QNetworkReply* m_reply;
    QNetworkAccessManager* m_manager;
};

#endif // UNITY_API_SCOPES_INTERNAL_SMARTSCOPES_HTTPCLIENTQTTHREAD_H

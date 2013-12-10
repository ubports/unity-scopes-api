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
#include <QtNetwork/QNetworkReply>
#include <QMutexLocker>
#include <QTimer>

HttpClientQtThread::HttpClientQtThread( const QUrl& url, const HttpHeadersList& headers )
    : QThread(),
      m_url( url ),
      m_headers( headers ),
      m_reply( nullptr ),
      m_manager( nullptr )
{
}

HttpClientQtThread::~HttpClientQtThread()
{
    if ( m_manager )
    {
        m_manager->deleteLater();
    }

    if ( m_reply )
    {
        m_reply->deleteLater();
    }
}

void HttpClientQtThread::run()
{
    m_manager = new QNetworkAccessManager();

    QNetworkRequest request( m_url );
    for ( auto it = m_headers.begin(); it != m_headers.end(); it++ )
    {
        request.setRawHeader( ( *it ).first, ( *it ).second );
    }

    connect( m_manager, SIGNAL( finished( QNetworkReply* ) ), this, SLOT( queryDone( QNetworkReply* ) ) );
    m_reply = m_manager->get( request );

    QTimer timeout;
    timeout.singleShot( 2000, this, SLOT( cancel() ) );

    QThread::exec(); // enter event loop
}

QNetworkReply* HttpClientQtThread::getReply() const
{
    return m_reply;
}

void HttpClientQtThread::queryDone( QNetworkReply* reply )
{
    quit();
}

void HttpClientQtThread::cancel()
{
    if ( m_reply )
    {
        m_reply->abort();
        quit();
    }
}

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

#include <QCoreApplication>
#include <QEventLoop>
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>

#include <iostream>

using namespace unity::api::scopes::internal::smartscopes;

Q_DECLARE_METATYPE( PromisePtr )

HttpClientQt::HttpClientQt()
{
    if ( !QCoreApplication::instance() )
    {
        int argc = 0;
        app_ = new QCoreApplication( argc, nullptr );
    }
}

HttpClientQt::~HttpClientQt()
{
    if ( get_thread_ )
    {
        get_thread_->join();
    }

    delete app_;
}

std::future<std::string> HttpClientQt::get( const std::string& request_url, int port )
{
    if ( get_thread_ )
    {
        get_thread_->join();
    }

    promise_ = std::make_shared<std::promise<std::string>>();

    get_thread_ = std::unique_ptr <std::thread> ( new std::thread( [&request_url, port, this]()
    {
        QUrl url( request_url.c_str() );
        url.setPort( port );

        auto thread = new HttpClientQtThread(url);
        QEventLoop loop;
        QObject::connect(thread, SIGNAL(finished()), &loop, SLOT(quit()));
        thread->start();
        loop.exec();
        thread->wait();

        QNetworkReply* reply = thread->getReply();
        thread->deleteLater();

        if ( !reply || reply->error() != QNetworkReply::NoError )
        {
            // communication error
            std::runtime_error e( reply->errorString().toStdString() );
            promise_->set_exception( make_exception_ptr( e ) );
        }
        else
        {
            QString reply_string( reply->readAll() );
            promise_->set_value( reply_string.toStdString() );
        }
    } ) );

    return promise_->get_future();
}

std::string HttpClientQt::to_percent_encoding( const std::string& string )
{
    return QUrl::toPercentEncoding( string.c_str() ).constData();
}

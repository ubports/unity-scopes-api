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

#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>

using namespace unity::api::scopes::internal::smartscopes;

using promise_ptr = std::shared_ptr< std::promise< std::string > >;
Q_DECLARE_METATYPE( promise_ptr )

HttpClientQt::HttpClientQt()
{
  connect( &network_manager_, SIGNAL( finished( QNetworkReply* ) ), this, SLOT( parse_network_response( QNetworkReply* ) ) );
}

HttpClientQt::~HttpClientQt()
{

}

std::future< std::string > HttpClientQt::get( std::string request_url )
{
  QUrl url( request_url.c_str() );
  QNetworkRequest q_request( url );
  QNetworkReply* current_reply = network_manager_.get( q_request );

  auto promise = std::make_shared< std::promise< std::string > >();
  current_reply->setProperty( "promise", QVariant::fromValue( promise ) );

  return promise->get_future();
}

void HttpClientQt::parse_network_response( QNetworkReply *reply )
{
  QVariant q_promise = reply->property( "promise" );
  auto promise = q_promise.value< promise_ptr >();

  if( reply->error() != QNetworkReply::NoError )
  {
    // communication error
    promise->set_value( "" );
    return;
  }

  // read string from reply
  QString reply_string( reply->readAll() );
  promise->set_value( reply_string.toStdString() );
}

#include "HttpClientQt.moc"

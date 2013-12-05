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

#include <scopes/internal/smartscopes/SmartScopesClient.h>

#include <future>
#include <iostream>
#include <algorithm>

using namespace unity::api::scopes::internal::smartscopes;

SmartScopesClient::SmartScopesClient( HttpClientInterface::SPtr http_client, JsonNodeInterface::SPtr json_node,
    const std::string& url, int port )
    : http_client_( http_client ),
      json_node_( json_node ),
      url_( url ),
      port_( port )
{

}

std::vector< RemoteScope > SmartScopesClient::get_remote_scopes()
{
  std::future< std::string > response = http_client_->get( url_ + "/smartscopes/v2/remote-scopes", port_ );
  response.wait();

  try
  {
    std::string response_str = response.get();
    if( !json_node_->read_json( response_str ) )
    {
      std::cout << "failed to parse json response from uri: " << url_ << "/smartscopes/v2/remote-scopes" << std::endl;
      return std::vector< RemoteScope >();
    }

    std::vector< RemoteScope > remote_scopes;
    JsonNodeInterface::SPtr scope_node;
    RemoteScope scope;
    std::string value;

    for( int i = 0; i < json_node_->size(); ++i )
    {
      json_node_->get_node( i, scope_node );

      scope_node->get_value( {"name"}, value );
      scope.name = value;

      scope_node->get_value( {"search_url"}, value );
      scope.search_url = value;

      scope_node->get_value( {"invisible"}, value );
      std::transform( value.begin(), value.end(), value.begin(), ::toupper );
      scope.invisible = value == "TRUE";

      remote_scopes.push_back( scope );
    }

    return remote_scopes;
  }
  catch( std::exception e )
  {
    std::cout << "failed to retrieve remote scopes from uri: " << url_ << "/smartscopes/v2/remote-scopes" << std::endl;
    std::cout << "error:" << e.what() << std::endl;
    return std::vector< RemoteScope >();
  }
}

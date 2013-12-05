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
  std::future< std::string > r = http_client_->get( url_, port_ );
  r.wait();

  return std::vector< RemoteScope >();
}

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

#ifndef UNITY_API_SCOPES_INTERNAL_SMARTSCOPES_SMARTSCOPESCLIENT_H
#define UNITY_API_SCOPES_INTERNAL_SMARTSCOPES_SMARTSCOPESCLIENT_H

#include <scopes/internal/smartscopes/HttpClientInterface.h>
#include <scopes/internal/smartscopes/JsonParserInterface.h>

#include <scopes/ResultItem.h>

#include <string>
#include <vector>
#include <memory>

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

namespace smartscopes
{

struct RemoteScope
{
  std::string name;
  std::string search_url;
  bool invisible = false;
};

class SmartScopesClient
{
public:
  UNITY_DEFINES_PTRS(SmartScopesClient);

  SmartScopesClient( HttpClientInterface::SPtr http_client,
      JsonNodeInterface::SPtr json_node,
      const std::string& url = "https://productsearch.ubuntu.com",
      int port = 80 );

  std::vector< RemoteScope > get_remote_scopes();

  std::future< std::vector< ResultItem > > search( const std::string& query,
      const std::string& session_id, const std::string& query_id,
      const std::string& platform, const std::string& locale = "",
      const std::string& country = "", const std::string& latitude = "",
      const std::string& longitude = "", const std::string& limit = "" );

private:
  HttpClientInterface::SPtr http_client_;
  JsonNodeInterface::SPtr json_node_;
  std::string url_;
  int port_;
};

} // namespace smartscopes

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

#endif // UNITY_API_SCOPES_INTERNAL_SMARTSCOPES_SMARTSCOPESCLIENT_H

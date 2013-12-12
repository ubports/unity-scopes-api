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
#include <unity/UnityExceptions.h>

#include <algorithm>
#include <future>
#include <iostream>
#include <map>

static const std::string c_base_url = "https://productsearch.ubuntu.com";
static const std::string c_remote_scopes_resourse = "/smartscopes/v2/remote-scopes";

using namespace unity::api::scopes;
using namespace unity::api::scopes::internal::smartscopes;

SmartScopesClient::SmartScopesClient( HttpClientInterface::SPtr http_client,
                                      JsonNodeInterface::SPtr json_node,
                                      const std::string& url, int port )
    : http_client_( http_client ),
      json_node_( json_node ),
      url_( url ),
      port_( port )
{
    if ( url_.empty() )
    {
        const char* base_url_env = ::getenv( "SMART_SCOPES_SERVER" );
        if ( base_url_env == nullptr )
        {
            url_ = c_base_url;
        }
        else
        {
            url_ = base_url_env;
        }
    }
}

SmartScopesClient::~SmartScopesClient()
{

}

std::vector<RemoteScope> SmartScopesClient::get_remote_scopes()
{
    try
    {
        std::string response_str;
        std::future<std::string> response = http_client_->get( url_ + c_remote_scopes_resourse, "", port_ );
        response.wait();

        response_str = response.get();

        std::lock_guard <std::mutex> lock( json_node_mutex_ );

        json_node_->read_json( response_str );

        std::vector<RemoteScope> remote_scopes;
        JsonNodeInterface::SPtr node;
        RemoteScope scope;

        for ( int i = 0; i < json_node_->size(); ++i )
        {
            node = json_node_->get_node( i );
            scope.name = node->has_node( "name" ) ?
                         node->get_node( "name" )->as_string() : "";
            scope.search_url = node->has_node( "search_url" ) ?
                               node->get_node( "search_url" )->as_string() : "";
            scope.invisible = node->has_node( "invisible" ) ?
                              node->get_node( "invisible" )->as_bool() : false;

            remote_scopes.push_back( scope );
        }

        return remote_scopes;
    }
    catch ( unity::Exception& e )
    {
        std::cout << "failed to retrieve remote scopes from uri: " << url_ << c_remote_scopes_resourse << std::endl;
        std::cout << "error:" << e.what() << std::endl;
        return std::vector<RemoteScope>();
    }
}

void SmartScopesClient::search( const std::string& scope_resourse, const std::string& query,
                                const std::string& session_id, uint query_id, const std::string& platform,
                                const std::string& locale, const std::string& country,
                                const std::string& latitude, const std::string& longitude, uint limit )
{
    //search_results_.wait();

    search_uri_ = url_ + scope_resourse + "?";

    // mandatory parameters

    search_uri_ += "query=\"" + http_client_->to_percent_encoding( query ) + "\"";
    search_uri_ += "&session_id=\"" + session_id + "\"";
    search_uri_ += "&query_id=" + std::to_string( query_id );
    search_uri_ += "&platform=\"" + platform + "\"";

    // optional parameters

    search_uri_ += locale.empty() ? "" : "&locale=\"" + locale + "\"";
    search_uri_ += country.empty() ? "" : "&country=\"" + country + "\"";
    search_uri_ += latitude.empty() ? "" : "&latitude=\"" + latitude + "\"";
    search_uri_ += longitude.empty() ? "" : "&longitude=\"" + longitude + "\"";
    search_uri_ += limit == 0 ? "" : "&limit=\"" + std::to_string( limit ) + "\"";

    search_results_ = http_client_->get( search_uri_, "", port_ );
}

std::vector<SearchResult> SmartScopesClient::get_search_results()
{
    try
    {
        std::string response_str;
        search_results_.wait();

        response_str = search_results_.get();

        std::vector<SearchResult> results;
        std::map<std::string, std::shared_ptr<SearchCategory>> categories;

        std::vector<std::string> jsons = extract_json_stream( response_str );

        std::lock_guard <std::mutex> lock( json_node_mutex_ );

        for ( std::string& json : jsons )
        {
            json_node_->read_json( json );

            JsonNodeInterface::SPtr node;
            std::string value;

            if ( json_node_->has_node( "category" ) )
            {
                node = json_node_->get_node( "category" );
                auto category = std::make_shared<SearchCategory>();

                category->icon = node->has_node( "icon" ) ?
                                 node->get_node( "icon" )->as_string() : "";
                category->id = node->has_node( "id" ) ?
                               node->get_node( "id" )->as_string() : "";
                category->renderer_template = node->has_node( "renderer_template" ) ?
                                              node->get_node( "renderer_template" )->as_string() : "";
                category->title = node->has_node( "title" ) ?
                                  node->get_node( "title" )->as_string() : "";
                categories[category->id] = category;
            }
            else if ( json_node_->has_node( "result" ) )
            {
                node = json_node_->get_node( "result" );
                SearchResult result;

                result.art = node->has_node( "art" ) ?
                             node->get_node( "art" )->as_string() : "";
                result.category = node->has_node( "cat_id" ) ?
                                  categories[node->get_node( "cat_id" )->as_string()] : nullptr;
                result.dnd_uri = node->has_node( "dnd_uri" ) ?
                                 node->get_node( "dnd_uri" )->as_string() : "";
                result.title = node->has_node( "title" ) ?
                               node->get_node( "title" )->as_string() : "";
                result.uri = node->has_node( "uri" ) ?
                             node->get_node( "uri" )->as_string() : "";

                results.push_back( result );
            }
        }

        return results;
    }
    catch ( unity::Exception& e )
    {
        std::cout << "failed to retrieve search results from uri: " << search_uri_ << std::endl;
        std::cout << "error:" << e.what() << std::endl;
        return std::vector<SearchResult>();
    }
}

std::vector<std::string> SmartScopesClient::extract_json_stream( const std::string& json_stream )
{
    std::vector<std::string> jsons;

    uint start_pos = 0;

    while ( start_pos < json_stream.size() )
    {
        int end_pos = json_stream.find( "\r\n", start_pos );
        if ( end_pos == -1 )
        {
            end_pos = json_stream.size();
        }

        std::string sub_json_str = json_stream.substr( start_pos, end_pos - start_pos );
        jsons.push_back( sub_json_str );
        start_pos = end_pos + 2;
    }

    return jsons;
}

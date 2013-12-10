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
    std::string response_str;
    std::future<std::string> response = http_client_->get( url_ + c_remote_scopes_resourse, port_ );
    response.wait();

    try
    {
        response_str = response.get();
    }
    catch ( std::exception& e )
    {
        std::cout << "failed to retrieve remote scopes from uri: " << url_ << c_remote_scopes_resourse << std::endl;
        std::cout << "error:" << e.what() << std::endl;
        return std::vector<RemoteScope>();
    }

    json_node_mutex_.lock();

    if ( !json_node_->read_json( response_str ) )
    {
        std::cout << "failed to parse json response from uri: " << url_ << c_remote_scopes_resourse << std::endl;
        return std::vector<RemoteScope>();
    }

    std::vector<RemoteScope> remote_scopes;
    JsonNodeInterface::SPtr scope_node;
    RemoteScope scope;
    std::string value;

    for ( int i = 0; i < json_node_->size(); ++i )
    {
        json_node_->get_node( i, scope_node );

        scope_node->get_value( { "name" }, value );
        scope.name = value;

        scope_node->get_value( { "search_url" }, value );
        scope.search_url = value;

        scope_node->get_value( { "invisible" }, value );
        std::transform( value.begin(), value.end(), value.begin(), ::toupper );
        scope.invisible = value == "TRUE";

        remote_scopes.push_back( scope );
    }

    json_node_mutex_.unlock();

    return remote_scopes;
}

void SmartScopesClient::search( const std::string& scope_resourse, const std::string& query,
                                const std::string& session_id, uint query_id, const std::string& platform,
                                const std::string& locale, const std::string& country,
                                const std::string& latitude, const std::string& longitude,
                                uint limit )
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

    search_results_ = http_client_->get( search_uri_, port_ );
}

std::vector<SearchResult> SmartScopesClient::get_search_results()
{
    std::string response_str;
    search_results_.wait();

    try
    {
        response_str = search_results_.get();
    }
    catch ( std::exception& e )
    {
        std::cout << "failed to retrieve search results from uri: " << search_uri_ << std::endl;
        std::cout << "error:" << e.what() << std::endl;
        return std::vector<SearchResult>();
    }

    std::vector<SearchResult> results;
    std::map<std::string, std::shared_ptr<SearchCategory>> categories;

    std::vector<std::string> jsons = extract_json_stream( response_str );

    json_node_mutex_.lock();

    for ( std::string& json : jsons )
    {
        if ( !json_node_->read_json( json ) )
        {
            break;
        }

        JsonNodeInterface::SPtr result_node;
        std::string value;

        if ( json_node_->get_node( {"category"}, result_node ) )
        {
            auto category = std::make_shared<SearchCategory>();

            result_node->get_value( {"icon"}, value );
            category->icon = value;
            result_node->get_value( {"id"}, value );
            category->id = value;
            result_node->get_value( {"renderer_template"}, value );
            category->renderer_template = value;
            result_node->get_value( {"title"}, value );
            category->title = value;
            categories[ category->id ] = category;
        }
        else if ( json_node_->get_node( {"result"}, result_node ) )
        {
            SearchResult result;

            result_node->get_value( {"art"}, value );
            result.art = value;
            result_node->get_value( {"cat_id"}, value );
            result.category = categories[value];
            result_node->get_value( {"dnd_uri"}, value );
            result.dnd_uri = value;
            result_node->get_value( {"title"}, value );
            result.title = value;
            result_node->get_value( {"uri"}, value );
            result.uri = value;

            results.push_back( result );
        }
    }

    json_node_mutex_.unlock();

    return results;
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

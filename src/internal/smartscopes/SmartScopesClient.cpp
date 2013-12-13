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
#include <sstream>

static const std::string c_base_url = "https://productsearch.ubuntu.com";
static const std::string c_remote_scopes_resourse = "/smartscopes/v2/remote-scopes";

using namespace unity::api::scopes;
using namespace unity::api::scopes::internal::smartscopes;

SmartScopesClient::SmartScopesClient(HttpClientInterface::SPtr http_client, JsonNodeInterface::SPtr json_node,
                                     const std::string& url, uint port)
    : http_client_(http_client),
      json_node_(json_node),
      url_(url),
      port_(port)
{
    if (url_.empty())
    {
        std::string base_url_env = ::getenv("SMART_SCOPES_SERVER");
        if (!base_url_env.empty())
        {
            uint64_t found = base_url_env.find_last_of(':');
            if ( found != std::string::npos && found > 5 )
            {
                url_ = base_url_env.substr(0, found);
                port_ = std::stoi( base_url_env.substr(found + 1) );
            }
            else
            {
                url_ = base_url_env;
            }
        }
        else
        {
            url_ = c_base_url;
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
        std::future<std::string> response = http_client_->get(url_ + c_remote_scopes_resourse, port_);
        response.wait();

        response_str = response.get();

        std::lock_guard <std::mutex> lock(json_node_mutex_);

        json_node_->read_json(response_str);

        std::vector<RemoteScope> remote_scopes;
        JsonNodeInterface::SPtr node;
        RemoteScope scope;

        for (int i = 0; i < json_node_->size(); ++i)
        {
            node = json_node_->get_node(i);
            scope.name = node->has_node("name") ?
                         node->get_node("name")->as_string() : "";
            scope.search_url = node->has_node("search_url") ?
                               node->get_node("search_url")->as_string() : "";
            scope.invisible = node->has_node("invisible") ?
                              node->get_node("invisible")->as_bool() : false;

            remote_scopes.push_back(scope);
        }

        return remote_scopes;
    }
    catch (unity::Exception& e)
    {
        std::cout << "failed to retrieve remote scopes from uri: " << url_ << c_remote_scopes_resourse << std::endl;
        std::cout << "error:" << e.what() << std::endl;
        return std::vector<RemoteScope>();
    }
}

void SmartScopesClient::search(const std::string& search_url, const std::string& query, const std::string& session_id,
                               uint query_id, const std::string& platform, const std::string& locale, const std::string& country,
                               const std::string& latitude, const std::string& longitude, uint limit)
{
    std::ostringstream search_uri;
    search_uri << search_url << "?";

    // mandatory parameters

    search_uri << "query=\"" << http_client_->to_percent_encoding(query) << "\"";
    search_uri << "&session_id=\"" << session_id << "\"";
    search_uri << "&query_id=" << std::to_string(query_id);
    search_uri << "&platform=\"" << platform << "\"";

    // optional parameters

    if ( !locale.empty() )
    {
        search_uri << "&locale=\"" << locale << "\"";
    }
    if ( !country.empty() )
    {
        search_uri << "&country=\"" << country << "\"";
    }
    if ( !latitude.empty() )
    {
        search_uri << "&latitude=\"" << latitude << "\"";
    }
    if ( !longitude.empty() )
    {
        search_uri << "&longitude=\"" << longitude << "\"";
    }
    if ( limit != 0 )
    {
        search_uri << "&limit=" << std::to_string(limit);
    }

    cancel_search(session_id);
    search_results_[session_id] = http_client_->get(search_uri.str(), port_);
}

void SmartScopesClient::cancel_search(const std::string& session_id)
{
    auto it = search_results_.find(session_id);
    if (it != search_results_.end())
    {
        http_client_->cancel_get(search_results_[session_id]);
        search_results_.erase(it);
    }
}

std::vector<SearchResult> SmartScopesClient::get_search_results(const std::string& session_id)
{
    try
    {
        auto it = search_results_.find(session_id);
        if (it == search_results_.end())
        {
            throw unity::LogicException("No search for session " + session_id + " was started");
        }

        std::string response_str;
        search_results_[session_id].wait();

        response_str = search_results_[session_id].get();
        search_results_.erase(it);

        std::vector<SearchResult> results;
        std::map<std::string, std::shared_ptr<SearchCategory>> categories;

        std::vector<std::string> jsons = extract_json_stream(response_str);

        std::lock_guard <std::mutex> lock(json_node_mutex_);

        for (std::string& json : jsons)
        {
            json_node_->read_json(json);

            JsonNodeInterface::SPtr node;
            std::string value;

            if (json_node_->has_node("category"))
            {
                node = json_node_->get_node("category");
                auto category = std::make_shared<SearchCategory>();

                category->icon = node->has_node("icon") ?
                                 node->get_node("icon")->as_string() : "";
                category->id = node->has_node("id") ?
                               node->get_node("id")->as_string() : "";
                category->renderer_template = node->has_node("renderer_template") ?
                                              node->get_node("renderer_template")->as_string() : "";
                category->title = node->has_node("title") ?
                                  node->get_node("title")->as_string() : "";
                categories[category->id] = category;
            }
            else if (json_node_->has_node("result"))
            {
                node = json_node_->get_node("result");
                SearchResult result;

                result.art = node->has_node("art") ?
                             node->get_node("art")->as_string() : "";
                result.category = node->has_node("cat_id") ?
                                  categories[node->get_node("cat_id")->as_string()] : nullptr;
                result.dnd_uri = node->has_node("dnd_uri") ?
                                 node->get_node("dnd_uri")->as_string() : "";
                result.title = node->has_node("title") ?
                               node->get_node("title")->as_string() : "";
                result.uri = node->has_node("uri") ?
                             node->get_node("uri")->as_string() : "";

                results.push_back(result);
            }
        }

        return results;
    }
    catch (unity::Exception& e)
    {
        std::cout << "failed to retrieve search results for session: " << session_id << std::endl;
        std::cout << "error:" << e.what() << std::endl;
        return std::vector<SearchResult>();
    }
}

std::vector<std::string> SmartScopesClient::extract_json_stream(const std::string& json_stream)
{
    std::vector<std::string> jsons;

    uint start_pos = 0;

    while (start_pos < json_stream.size())
    {
        int end_pos = json_stream.find("\r\n", start_pos);
        if (end_pos == -1)
        {
            end_pos = json_stream.size();
        }

        std::string sub_json_str = json_stream.substr(start_pos, end_pos - start_pos);
        jsons.push_back(sub_json_str);
        start_pos = end_pos + 2;
    }

    return jsons;
}

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

#include <unity/scopes/internal/smartscopes/SmartScopesClient.h>
#include <unity/UnityExceptions.h>

#include <algorithm>
#include <future>
#include <iostream>
#include <map>
#include <sstream>
#include <cstring>

static const std::string c_base_url = "https://productsearch.ubuntu.com";
static const std::string c_remote_scopes_resource = "/smartscopes/v2/remote-scopes";

using namespace unity::scopes;
using namespace unity::scopes::internal::smartscopes;

//-- SearchHandle

SearchHandle::SearchHandle(std::string const& session_id, SmartScopesClient::SPtr ssc)
    : session_id_(session_id),
      ssc_(ssc) {}

SearchHandle::~SearchHandle()
{
    ssc_->cancel_search(session_id_);
}

std::vector<SearchResult> SearchHandle::get_search_results()
{
    return ssc_->get_search_results(session_id_);
}

//-- SmartScopesClient

SmartScopesClient::SmartScopesClient(HttpClientInterface::SPtr http_client, JsonNodeInterface::SPtr json_node,
                                     std::string const& url, uint port)
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
            // find the last occurrence of ':' in the url in order to extract the port number
            // * ignore the colon after "http"/"https"

            const size_t hier_pos = strlen("https");

            uint64_t found = base_url_env.find_last_of(':');
            if (found != std::string::npos && found > hier_pos)
            {
                url_ = base_url_env.substr(0, found);
                port_ = std::stoi(base_url_env.substr(found + 1));
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
        HttpResponseHandle::SPtr response = http_client_->get(url_ + c_remote_scopes_resource, port_);
        response->wait();

        response_str = response->get();

        std::vector<RemoteScope> remote_scopes;
        JsonNodeInterface::SPtr root_node;
        JsonNodeInterface::SPtr child_node;
        RemoteScope scope;

        {
            std::lock_guard<std::mutex> lock(json_node_mutex_);
            json_node_->read_json(response_str);
            root_node = json_node_->get_node();
        }

        for (int i = 0; i < root_node->size(); ++i)
        {
            child_node = root_node->get_node(i);

            if (!child_node->has_node("name") || !child_node->has_node("search_url"))
            {
                break;
            }

            scope.name = child_node->get_node("name")->as_string();
            scope.search_url = child_node->get_node("search_url")->as_string();

            scope.invisible = child_node->has_node("invisible") ?
                              child_node->get_node("invisible")->as_bool() : false;

            remote_scopes.push_back(scope);
        }

        return remote_scopes;
    }
    catch (unity::Exception const& e)
    {
        std::cout << "failed to retrieve remote scopes from uri: " << url_ << c_remote_scopes_resource << std::endl;
        std::cout << "error:" << e.what() << std::endl;
        return std::vector<RemoteScope>();
    }
}

SearchHandle::UPtr SmartScopesClient::search(std::string const& search_url, std::string const& query,
                                             std::string const& session_id, uint query_id, std::string const& platform,
                                             std::string const& locale, std::string const& country,
                                             std::string const& latitude, std::string const& longitude,
                                             uint limit)
{
    std::ostringstream search_uri;
    search_uri << search_url << "?";

    // mandatory parameters

    search_uri << "query=\"" << http_client_->to_percent_encoding(query) << "\"";
    search_uri << "&session_id=\"" << session_id << "\"";
    search_uri << "&query_id=" << std::to_string(query_id);
    search_uri << "&platform=\"" << platform << "\"";

    // optional parameters

    if (!locale.empty())
    {
        search_uri << "&locale=\"" << locale << "\"";
    }
    if (!country.empty())
    {
        search_uri << "&country=\"" << country << "\"";
    }
    if (!latitude.empty())
    {
        search_uri << "&latitude=\"" << latitude << "\"";
    }
    if (!longitude.empty())
    {
        search_uri << "&longitude=\"" << longitude << "\"";
    }
    if (limit != 0)
    {
        search_uri << "&limit=" << std::to_string(limit);
    }

    cancel_search(session_id);

    std::lock_guard<std::mutex> lock(search_results_mutex_);
    search_results_[session_id] = http_client_->get(search_uri.str(), port_);

    return SearchHandle::UPtr(new SearchHandle(session_id, shared_from_this()));
}

std::vector<SearchResult> SmartScopesClient::get_search_results(std::string const& session_id)
{
    try
    {
        std::string response_str;

        {
            std::lock_guard<std::mutex> lock(search_results_mutex_);

            auto it = search_results_.find(session_id);
            if (it == search_results_.end())
            {
                throw unity::LogicException("No search for session " + session_id + " is active");
            }

            search_results_[session_id]->wait();

            response_str = search_results_[session_id]->get();
            search_results_.erase(it);
        }

        std::vector<SearchResult> results;
        std::map<std::string, std::shared_ptr<SearchCategory>> categories;

        std::vector<std::string> jsons = extract_json_stream(response_str);

        for (std::string& json : jsons)
        {
            JsonNodeInterface::SPtr root_node;
            JsonNodeInterface::SPtr child_node;

            {
                std::lock_guard<std::mutex> lock(json_node_mutex_);
                json_node_->read_json(json);
                root_node = json_node_->get_node();
            }

            if (root_node->has_node("category"))
            {
                child_node = root_node->get_node("category");
                auto category = std::make_shared<SearchCategory>();

                category->icon = child_node->has_node("icon") ?
                                 child_node->get_node("icon")->as_string() : "";
                category->id = child_node->has_node("id") ?
                               child_node->get_node("id")->as_string() : "";
                category->renderer_template = child_node->has_node("renderer_template") ?
                                              child_node->get_node("renderer_template")->as_string() : "";
                category->title = child_node->has_node("title") ?
                                  child_node->get_node("title")->as_string() : "";
                categories[category->id] = category;
            }
            else if (root_node->has_node("result"))
            {
                child_node = root_node->get_node("result");
                SearchResult result;

                result.art = child_node->has_node("art") ?
                             child_node->get_node("art")->as_string() : "";
                result.dnd_uri = child_node->has_node("dnd_uri") ?
                                 child_node->get_node("dnd_uri")->as_string() : "";
                result.title = child_node->has_node("title") ?
                               child_node->get_node("title")->as_string() : "";
                result.uri = child_node->has_node("uri") ?
                             child_node->get_node("uri")->as_string() : "";

                std::string category = child_node->has_node("cat_id") ?
                                       child_node->get_node("cat_id")->as_string() : "";

                result.category = categories.find(category) != categories.end() ?
                                  categories[category] : nullptr;

                results.push_back(result);
            }
        }

        return results;
    }
    catch (unity::Exception const& e)
    {
        std::cout << "failed to retrieve search results for session: " << session_id << std::endl;
        std::cout << "error:" << e.what() << std::endl;
        return std::vector<SearchResult>();
    }
}

std::vector<std::string> SmartScopesClient::extract_json_stream(std::string const& json_stream)
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

void SmartScopesClient::cancel_search(std::string const& session_id)
{
    std::lock_guard<std::mutex> lock(search_results_mutex_);

    auto it = search_results_.find(session_id);
    if (it != search_results_.end())
    {
        http_client_->cancel_get(search_results_[session_id]);
        search_results_.erase(it);
    }
}

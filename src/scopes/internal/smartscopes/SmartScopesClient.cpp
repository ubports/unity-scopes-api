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

static const std::string c_base_url = "https://productsearch.ubuntu.com/smartscopes/v2";
static const std::string c_remote_scopes_resource = "/remote-scopes";
static const std::string c_search_resource = "/search";
static const std::string c_preview_resource = "/preview";

using namespace unity::scopes;
using namespace unity::scopes::internal::smartscopes;

//-- SearchHandle

SearchHandle::SearchHandle(std::string const& session_id, SmartScopesClient::SPtr ssc)
    : session_id_(session_id)
    , ssc_(ssc)
{
}

SearchHandle::~SearchHandle()
{
    cancel_search();
}

std::vector<SearchResult> SearchHandle::get_search_results()
{
    return ssc_->get_search_results(session_id_);
}

void SearchHandle::cancel_search()
{
    ssc_->cancel_search(session_id_);
}

//-- PreviewHandle

PreviewHandle::PreviewHandle(std::string const& session_id, SmartScopesClient::SPtr ssc)
    : session_id_(session_id)
    , ssc_(ssc)
{
}

PreviewHandle::~PreviewHandle()
{
    cancel_preview();
}

std::pair<PreviewHandle::Columns, PreviewHandle::Widgets> PreviewHandle::get_preview_results()
{
    return ssc_->get_preview_results(session_id_);
}

void PreviewHandle::cancel_preview()
{
    ssc_->cancel_preview(session_id_);
}

//-- SmartScopesClient

SmartScopesClient::SmartScopesClient(HttpClientInterface::SPtr http_client,
                                     JsonNodeInterface::SPtr json_node,
                                     std::string const& url,
                                     uint port)
    : http_client_(http_client)
    , json_node_(json_node)
    , url_(url)
    , port_(port)
{
    if (url_.empty())
    {
        char* base_url_env = ::getenv("SMART_SCOPES_SERVER");
        std::string base_url = base_url_env ? base_url_env : "";
        if (!base_url.empty())
        {
            // find the last occurrence of ':' in the url in order to extract the port number
            // * ignore the colon after "http"/"https"

            const size_t hier_pos = strlen("https");

            uint64_t found = base_url.find_last_of(':');
            if (found != std::string::npos && found > hier_pos)
            {
                url_ = base_url.substr(0, found);
                port_ = std::stoi(base_url.substr(found + 1));
            }
            else
            {
                url_ = base_url;
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

std::vector<RemoteScope> SmartScopesClient::get_remote_scopes(std::string const& locale)
{
    try
    {
        std::ostringstream remote_scopes_uri;
        remote_scopes_uri << url_ << c_remote_scopes_resource << "?";

        // optional parameters

        if (!locale.empty())
        {
            remote_scopes_uri << "&locale=\"" << locale << "\"";
        }

        std::string response_str;
        std::cout << "SmartScopesClient.get_remote_scopes(): GET " << remote_scopes_uri.str() << std::endl;
        HttpResponseHandle::SPtr response = http_client_->get(remote_scopes_uri.str(), port_);
        response->wait();

        response_str = response->get();
        std::cout << "SmartScopesClient.get_remote_scopes(): Remote scopes:" << std::endl << response_str << std::endl;

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

            if (!child_node->has_node("name") || !child_node->has_node("base_url") ||
                !child_node->has_node("description"))
            {
                break;
            }

            scope.name = child_node->get_node("name")->as_string();
            scope.description = child_node->get_node("description")->as_string();
            scope.base_url = child_node->get_node("base_url")->as_string();

            scope.invisible = child_node->has_node("invisible") ? child_node->get_node("invisible")->as_bool() : false;

            remote_scopes.push_back(scope);
        }

        std::cout << "SmartScopesClient.get_remote_scopes(): Retrieved remote scopes from uri: "
                  << url_ << c_remote_scopes_resource << std::endl;
        return remote_scopes;
    }
    catch (unity::Exception const& e)
    {
        std::cerr << "SmartScopesClient.get_remote_scopes(): Failed to retrieve remote scopes from uri: "
                  << url_ << c_remote_scopes_resource << std::endl;
        throw;
    }
}

SearchHandle::UPtr SmartScopesClient::search(std::string const& base_url,
                                             std::string const& query,
                                             std::string const& session_id,
                                             uint query_id,
                                             std::string const& platform,
                                             std::string const& locale,
                                             std::string const& country,
                                             uint limit)
{
    std::ostringstream search_uri;
    search_uri << base_url << c_search_resource << "?";

    // mandatory parameters

    search_uri << "q=" << http_client_->to_percent_encoding(query);
    search_uri << "&session_id=" << session_id;
    search_uri << "&query_id=" << std::to_string(query_id);
    search_uri << "&platform=" << platform;

    // optional parameters

    if (!locale.empty())
    {
        search_uri << "&locale=" << locale;
    }
    if (!country.empty())
    {
        search_uri << "&country=" << country;
    }
    if (limit != 0)
    {
        search_uri << "&limit=" << std::to_string(limit);
    }

    cancel_search(session_id);

    std::lock_guard<std::mutex> lock(search_results_mutex_);
    std::cout << "SmartScopesClient.search(): GET " << search_uri.str() << std::endl;
    search_results_[session_id] = http_client_->get(search_uri.str(), port_);

    return SearchHandle::UPtr(new SearchHandle(session_id, shared_from_this()));
}

PreviewHandle::UPtr SmartScopesClient::preview(std::string const& base_url,
                                               std::string const& result,
                                               std::string const& session_id,
                                               std::string const& platform,
                                               const uint widgets_api_version,
                                               std::string const& locale,
                                               std::string const& country)
{
    std::ostringstream preview_uri;
    preview_uri << base_url << c_preview_resource << "?";

    // mandatory parameters

    preview_uri << "result=" << http_client_->to_percent_encoding(result);
    preview_uri << "&session_id=" << session_id;
    preview_uri << "&platform=" << platform;
    preview_uri << "&widgets_api_version=" << std::to_string(widgets_api_version);

    // optional parameters

    if (!locale.empty())
    {
        preview_uri << "&locale=" << locale;
    }
    if (!country.empty())
    {
        preview_uri << "&country=" << country;
    }

    cancel_preview(session_id);

    std::lock_guard<std::mutex> lock(preview_results_mutex_);
    std::cout << "SmartScopesClient.preview(): GET " << preview_uri.str() << std::endl;
    preview_results_[session_id] = http_client_->get(preview_uri.str(), port_);

    return PreviewHandle::UPtr(new PreviewHandle(session_id, shared_from_this()));
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
            std::cout << "SmartScopesClient.get_search_results():" << std::endl << response_str << std::endl;
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

                std::vector<std::string> members = child_node->member_names();
                for (auto& member : members)
                {
                    if (member == "icon")
                    {
                        category->icon = child_node->get_node(member)->as_string();
                    }
                    else if (member == "id")
                    {
                        category->id = child_node->get_node(member)->as_string();
                    }
                    else if (member == "render_template")
                    {
                        category->renderer_template = child_node->get_node(member)->as_string();
                    }
                    else if (member == "title")
                    {
                        category->title = child_node->get_node(member)->as_string();
                    }
                }

                categories[category->id] = category;
            }
            else if (root_node->has_node("result"))
            {
                child_node = root_node->get_node("result");
                SearchResult result;
                result.json = child_node->to_json_string();

                std::vector<std::string> members = child_node->member_names();
                for (auto& member : members)
                {
                    if (member == "art")
                    {
                        result.art = child_node->get_node(member)->as_string();
                    }
                    else if (member == "dnd_uri")
                    {
                        result.dnd_uri = child_node->get_node(member)->as_string();
                    }
                    else if (member == "title")
                    {
                        result.title = child_node->get_node(member)->as_string();
                    }
                    else if (member == "uri")
                    {
                        result.uri = child_node->get_node(member)->as_string();
                    }
                    else if (member == "cat_id")
                    {
                        std::string category = child_node->get_node(member)->as_string();
                        result.category = categories.find(category) != categories.end() ? categories[category] : nullptr;
                    }
                    else
                    {
                        result.other_params[member] = child_node->get_node(member);
                    }
                }

                results.push_back(result);
            }
        }

        std::cout << "SmartScopesClient.get_search_results(): Retrieved search results for session: "
                  << session_id << std::endl;
        return results;
    }
    catch (unity::Exception const& e)
    {
        std::cerr << "SmartScopesClient.get_search_results(): Failed to retrieve search results for session: "
                  << session_id << std::endl;
        throw;
    }
}

std::pair<PreviewHandle::Columns, PreviewHandle::Widgets> SmartScopesClient::get_preview_results(std::string const& session_id)
{
    try
    {
        std::string response_str;

        {
            std::lock_guard<std::mutex> lock(preview_results_mutex_);

            auto it = preview_results_.find(session_id);
            if (it == preview_results_.end())
            {
                throw unity::LogicException("No preivew for session " + session_id + " is active");
            }

            preview_results_[session_id]->wait();

            response_str = preview_results_[session_id]->get();
            std::cout << "SmartScopesClient.get_preview_results():" << std::endl << response_str << std::endl;
            preview_results_.erase(it);
        }

        PreviewHandle::Columns columns;
        PreviewHandle::Widgets widgets;

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

            if (root_node->has_node("columns"))
            {
                child_node = root_node->get_node("columns");

                // for each column
                for (int column_i = 0; column_i < child_node->size(); ++column_i)
                {
                    auto column_node = child_node->get_node(column_i);

                    // for each widget layout within the column
                    std::vector<std::vector<std::string>> widget_layouts;
                    for (int widget_lo_i = 0; widget_lo_i < column_node->size(); ++widget_lo_i)
                    {
                        auto widget_lo_node = column_node->get_node(widget_lo_i);

                        // for each widget within the widget layout
                        std::vector<std::string> widgets;
                        for (int widget_i = 0; widget_i < widget_lo_node->size(); ++widget_i)
                        {
                            auto widget_node = widget_lo_node->get_node(widget_i);
                            widgets.push_back(widget_node->as_string());
                        }

                        widget_layouts.push_back(widgets);
                    }

                    columns.push_back(widget_layouts);
                }
            }
            else if (root_node->has_node("widget"))
            {
                child_node = root_node->get_node("widget");
                widgets.push_back(child_node->to_json_string());
            }
        }

        std::cout << "SmartScopesClient.get_preview_results(): Retrieved preview results for session: "
                  << session_id << std::endl;
        return std::make_pair(columns, widgets);
    }
    catch (unity::Exception const& e)
    {
        std::cerr << "SmartScopesClient.get_preview_results(): Failed to retrieve preview results for session: "
                  << session_id << std::endl;
        throw;
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

void SmartScopesClient::cancel_preview(std::string const& session_id)
{
    std::lock_guard<std::mutex> lock(preview_results_mutex_);

    auto it = preview_results_.find(session_id);
    if (it != preview_results_.end())
    {
        http_client_->cancel_get(preview_results_[session_id]);
        preview_results_.erase(it);
    }
}

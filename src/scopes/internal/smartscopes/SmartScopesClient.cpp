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
 *              Pawel Stolowski <pawel.stolowski@canonical.com>
 */

#include <unity/scopes/internal/FilterBaseImpl.h>
#include <unity/scopes/internal/FilterStateImpl.h>
#include <unity/scopes/internal/smartscopes/SmartScopesClient.h>

#include <unity/scopes/ScopeExceptions.h>
#include <unity/UnityExceptions.h>

#include <algorithm>
#include <array>
#include <cstring>
#include <fstream>
#include <future>
#include <utility>
#include <iostream>
#include <map>
#include <sstream>
#include <sys/stat.h>

static std::string homedir()
{
    static const char* home = getenv("HOME");
    if (!home)
    {
        throw unity::scopes::ConfigException("smartscopesclient: HOME not set, cannot configure cache");
    }
    return home;
}

static const std::string c_base_url = "https://dash.ubuntu.com/smartscopes/v2";
static const std::string c_remote_scopes_resource = "/remote-scopes";
static const std::string c_search_resource = "/search";
static const std::string c_preview_resource = "/preview";

static const std::string c_scopes_cache_dir = homedir() + "/.cache/unity-scopes/";
static const std::string c_scopes_cache_filename = "remote-scopes.json";

using namespace unity::scopes;
using namespace unity::scopes::internal::smartscopes;

//-- SearchHandle

SearchHandle::SearchHandle(uint search_id, SmartScopesClient::SPtr ssc)
    : search_id_(search_id)
    , ssc_(ssc)
{
}

SearchHandle::~SearchHandle()
{
    cancel_search();
}

void SearchHandle::wait()
{
    ssc_->wait_for_search(search_id_);
}

void SearchHandle::cancel_search()
{
    ssc_->cancel_query(search_id_);
}

//-- PreviewHandle

PreviewHandle::PreviewHandle(uint preview_id, SmartScopesClient::SPtr ssc)
    : preview_id_(preview_id)
    , ssc_(ssc)
{
}

PreviewHandle::~PreviewHandle()
{
    cancel_preview();
}

void PreviewHandle::wait()
{
    ssc_->wait_for_preview(preview_id_);
}

void PreviewHandle::cancel_preview()
{
    ssc_->cancel_query(preview_id_);
}

//-- SmartScopesClient

SmartScopesClient::SmartScopesClient(HttpClientInterface::SPtr http_client,
                                     JsonNodeInterface::SPtr json_node,
                                     std::string const& url)
    : http_client_(http_client)
    , json_node_(json_node)
    , have_latest_cache_(false)
    , query_counter_(0)
{
    // initialise url_
    reset_url(url);

    // initialise cached_scopes_
    read_cache();
}

SmartScopesClient::~SmartScopesClient()
{
}

void SmartScopesClient::reset_url(std::string const& url)
{
    std::string base_url = url;

    // if a url was not provided, get the environment variable
    if (base_url.empty())
    {
        char* sss_url_env = ::getenv("SMART_SCOPES_SERVER");
        base_url = sss_url_env ? sss_url_env : "";

        // if the env var was not provided, use the c_base_url constant
        if (base_url.empty())
        {
            base_url = c_base_url;
        }
    }

    url_ = base_url;
}

std::string SmartScopesClient::url()
{
    return url_;
}

// returns false if cache used
bool SmartScopesClient::get_remote_scopes(std::vector<RemoteScope>& remote_scopes, std::string const& locale, bool caching_enabled)
{
    std::string response_str;
    bool using_cache = false;

    try
    {
        std::ostringstream remote_scopes_uri;
        remote_scopes_uri << url_ << c_remote_scopes_resource << "?";

        // optional parameters

        if (!locale.empty())
        {
            remote_scopes_uri << "locale=" << locale;
        }

        std::cout << "SmartScopesClient.get_remote_scopes(): GET " << remote_scopes_uri.str() << std::endl;

        HttpResponseHandle::SPtr response = http_client_->get(remote_scopes_uri.str(), [&response_str](std::string const& replyLine) {
                response_str += replyLine; // accumulate all reply lines
        });
        response->wait();

        std::cout << "SmartScopesClient.get_remote_scopes(): Remote scopes:" << std::endl << response_str << std::endl;
    }
    catch (std::exception const& e)
    {
        std::cerr << "SmartScopesClient.get_remote_scopes(): Failed to retrieve remote scopes from uri: "
                  << url_ << c_remote_scopes_resource << std::endl;

        if (caching_enabled)
        {
            std::cerr << e.what() << std::endl;
            std::cerr << "SmartScopesClient.get_remote_scopes(): Using remote scopes from cache" << std::endl;

            response_str = read_cache();
            if (response_str.empty())
            {
                throw ResourceException("SmartScopesClient.get_remote_scopes(): Remote scopes cache is empty");
            }

            using_cache = true;
        }
        else
        {
            throw;
        }
    }

    JsonNodeInterface::SPtr root_node;
    JsonNodeInterface::SPtr child_node;

    try
    {
        std::lock_guard<std::mutex> lock(json_node_mutex_);
        json_node_->read_json(response_str);
        root_node = json_node_->get_node();
    }
    catch (std::exception const&)
    {
        std::cerr << "SmartScopesClient.get_remote_scopes() Failed to parse json response from uri: "
                  << url_ << c_remote_scopes_resource << std::endl;
        throw;
    }

    remote_scopes.clear();
    for (int i = 0; i < root_node->size(); ++i)
    {
        RemoteScope scope;

        try
        {
            child_node = root_node->get_node(i);

            if (!child_node->has_node("id"))
            {
                std::cerr << "SmartScopesClient.get_remote_scopes(): Skipping scope with no id" << std::endl;
                continue;
            }

            scope.id = child_node->get_node("id")->as_string();

            bool err = false;
            static std::array<char const*, 4> const mandatory = { { "name", "description", "author", "base_url" } };
            for (auto const& field : mandatory)
            {
                if (!child_node->has_node(field))
                {
                    std::cerr << "SmartScopesClient.get_remote_scopes(): Scope: \"" << scope.id
                              << "\" has no \"" << field << "\" field" << std::endl;
                    err = true;
                }
            }
            if (err)
            {
                std::cerr << "SmartScopesClient.get_remote_scopes(): Skipping scope: \""
                          << scope.id << "\"" << std::endl;
                continue;
            }

            scope.name = child_node->get_node("name")->as_string();
            scope.description = child_node->get_node("description")->as_string();
            scope.author = child_node->get_node("author")->as_string();
            scope.base_url = child_node->get_node("base_url")->as_string();

            if (child_node->has_node("icon"))
            {
                scope.icon.reset(new std::string(child_node->get_node("icon")->as_string()));
            }

            if (child_node->has_node("art"))
            {
                scope.art.reset(new std::string(child_node->get_node("art")->as_string()));
            }

            if (child_node->has_node("appearance"))
            {
                scope.appearance.reset(new VariantMap(child_node->get_node("appearance")->to_variant().get_dict()));
            }

            if (child_node->has_node("settings"))
            {
                scope.settings.reset(new std::string(child_node->get_node("settings")->to_json_string()));
            }

            if (child_node->has_node("needs_location_data"))
            {
                scope.needs_location_data.reset(new bool(child_node->get_node("needs_location_data")->as_bool()));
            }

            scope.invisible = child_node->has_node("invisible") ? child_node->get_node("invisible")->as_bool() : false;

            remote_scopes.push_back(scope);
        }
        catch (std::exception const& e)
        {
            std::cerr << "SmartScopesClient.get_remote_scopes(): Skipping scope: \""
                      << scope.id << "\" due to a json parsing failure" << std::endl;
            std::cerr << e.what() << std::endl;
        }
    }

    if (remote_scopes.empty())
    {
        std::cerr << "SmartScopesClient.get_remote_scopes(): No valid remote scopes retrieved from uri: "
                  << url_ << c_remote_scopes_resource << std::endl;
    }
    else if (!using_cache)
    {
        if (caching_enabled)
        {
            try
            {
                write_cache(response_str);
            }
            catch (std::exception const& e)
            {
                std::cerr << "SmartScopesClient.get_remote_scopes(): Failed to write to cache file: "
                          << c_scopes_cache_dir << c_scopes_cache_filename << std::endl;
                std::cerr << e.what() << std::endl;
            }
        }

        std::cout << "SmartScopesClient.get_remote_scopes(): Retrieved remote scopes from uri: "
                  << url_ << c_remote_scopes_resource << std::endl;
    }
    else
    {
        std::cout << "SmartScopesClient.get_remote_scopes(): Retrieved remote scopes from cache" << std::endl;
    }

    return !using_cache;
}

SearchHandle::UPtr SmartScopesClient::search(SearchReplyHandler const& handler,
                                             std::string const& base_url,
                                             std::string const& query,
                                             std::string const& department_id,
                                             std::string const& session_id,
                                             int query_id,
                                             std::string const& platform,
                                             VariantMap const& settings,
                                             VariantMap const& filter_state,
                                             std::string const& locale,
                                             std::string const& country,
                                             std::string const& user_agent_hdr,
                                             uint limit)
{
    std::ostringstream search_uri;
    search_uri << base_url << c_search_resource << "?";

    // mandatory parameters
    search_uri << "q=" << http_client_->to_percent_encoding(query);

    search_uri << "&session_id=" << http_client_->to_percent_encoding(session_id);
    search_uri << "&query_id=" << std::to_string(query_id);
    search_uri << "&platform=" << platform;

    // optional parameters
    if (!department_id.empty())
    {
        search_uri << "&department=" << http_client_->to_percent_encoding(department_id);
    }
    if (!settings.empty())
    {
        std::string settings_str = stringify_settings(settings);
        if (!settings_str.empty())
        {
            search_uri << "&settings=" << http_client_->to_percent_encoding(settings_str);
        }
    }
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
    if (!filter_state.empty())
    {
        search_uri << "&filters=" << http_client_->to_percent_encoding(Variant(filter_state).serialize_json());
    }

    std::lock_guard<std::mutex> lock(query_results_mutex_);
    uint search_id = ++query_counter_;

    std::cout << "SmartScopesClient.search(): GET " << search_uri.str() << std::endl;

    HttpHeaders headers;
    if (!user_agent_hdr.empty())
    {
        std::cout << "User agent: " << user_agent_hdr;
        headers.push_back(std::make_pair("User-Agent", user_agent_hdr));
    }

    query_results_[search_id] = http_client_->get(search_uri.str(), [this, handler](std::string const& lineData) {
            try
            {
                parse_line(lineData, handler);
            }
            catch (std::exception const &e)
            {
                std::cerr << "Failed to parse: " << e.what() << std::endl;
            }
    }, headers);

    return SearchHandle::UPtr(new SearchHandle(search_id, shared_from_this()));
}

PreviewHandle::UPtr SmartScopesClient::preview(PreviewReplyHandler const& handler,
                                               std::string const& base_url,
                                               std::string const& result,
                                               std::string const& session_id,
                                               std::string const& platform,
                                               const uint widgets_api_version,
                                               VariantMap const& settings,
                                               std::string const& locale,
                                               std::string const& user_agent_hdr,
                                               std::string const& country)
{
    std::ostringstream preview_uri;
    preview_uri << base_url << c_preview_resource << "?";

    // mandatory parameters

    preview_uri << "result=" << http_client_->to_percent_encoding(result);
    preview_uri << "&session_id=" << http_client_->to_percent_encoding(session_id);
    preview_uri << "&platform=" << platform;
    preview_uri << "&widgets_api_version=" << std::to_string(widgets_api_version);

    HttpHeaders headers;
    if (!user_agent_hdr.empty())
    {
        headers.push_back(std::make_pair("User-Agent", user_agent_hdr));
    }

    // optional parameters

    if (!settings.empty())
    {
        std::string settings_str = stringify_settings(settings);
        if (!settings_str.empty())
        {
            preview_uri << "&settings=" << http_client_->to_percent_encoding(settings_str);
        }
    }
    if (!locale.empty())
    {
        preview_uri << "&locale=" << locale;
    }
    if (!country.empty())
    {
        preview_uri << "&country=" << country;
    }

    std::lock_guard<std::mutex> lock(query_results_mutex_);
    uint preview_id = ++query_counter_;

    std::cout << "SmartScopesClient.preview(): GET " << preview_uri.str() << std::endl;
    query_results_[preview_id] = http_client_->get(preview_uri.str(), [this, handler](std::string const& lineData) {
            try
            {
                parse_line(lineData, handler);
            }
            catch (std::exception const &e)
            {
                std::cerr << "Failed to parse: " << e.what() << std::endl;
            }
    }, headers);

    return PreviewHandle::UPtr(new PreviewHandle(preview_id, shared_from_this()));
}

void SmartScopesClient::parse_line(std::string const& json, PreviewReplyHandler const& handler)
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
        PreviewHandle::Columns columns;
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
                std::vector<std::string> widget_ids;
                for (int widget_i = 0; widget_i < widget_lo_node->size(); ++widget_i)
                {
                    auto widget_node = widget_lo_node->get_node(widget_i);
                    widget_ids.push_back(widget_node->as_string());
                }

                widget_layouts.push_back(widget_ids);
            }

            columns.push_back(widget_layouts);
        }
        handler.columns_handler(columns);
    }
    else if (root_node->has_node("widget"))
    {
        child_node = root_node->get_node("widget");
        handler.widget_handler(child_node->to_json_string());
    }
}

void SmartScopesClient::parse_line(std::string const& json, SearchReplyHandler const& handler)
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
        handler.category_handler(category);
    }
    else if (root_node->has_node("result"))
    {
        child_node = root_node->get_node("result");
        SearchResult result;
        result.json = child_node->to_json_string();

        std::vector<std::string> members = child_node->member_names();
        for (auto& member : members)
        {
            if (member == "uri")
            {
                result.uri = child_node->get_node(member)->as_string();
            }
            else if (member == "cat_id")
            {
                std::string category = child_node->get_node(member)->as_string();
                result.category_id = category;
            }
            else
            {
                result.other_params[member] = child_node->get_node(member);
            }
        }
        handler.result_handler(result);
    }
    else if (root_node->has_node("departments"))
    {
        auto departments = parse_departments(root_node->get_node("departments"));
        handler.departments_handler(departments);
    }
    else if (root_node->has_node("filters"))
    {
        auto filters = parse_filters(root_node->get_node("filters"));
        handler.filters_handler(filters);
    }
    else if (root_node->has_node("filter_state"))
    {
        auto filter_state = parse_filter_state(root_node->get_node("filter_state"));
        handler.filter_state_handler(filter_state);
    }
}

void SmartScopesClient::wait_for_search(uint search_id)
{
    try
    {
        HttpResponseHandle::SPtr query_result;
        {
            std::lock_guard<std::mutex> lock(query_results_mutex_);

            auto it = query_results_.find(search_id);
            if (it == query_results_.end())
            {
                throw unity::LogicException("No search for query " + std::to_string(search_id) + " is active");
            }

            query_result = it->second;
        }

        query_result->wait();
        query_result->get(); // may throw on error
    }
    catch (std::exception const& e)
    {
        std::cerr << "SmartScopesClient.get_search_results(): Failed to retrieve search results for query " << search_id << ": " << e.what() << std::endl;
        throw;
    }

    std::lock_guard<std::mutex> lock(query_results_mutex_);
    query_results_.erase(search_id);
}

std::shared_ptr<DepartmentInfo> SmartScopesClient::parse_departments(JsonNodeInterface::SPtr node)
{
    static std::array<std::string, 2> const mandatory = { { "label", "canned_query" } };
    for (auto const& field : mandatory)
    {
        if (!node->has_node(field))
        {
            std::stringstream err;
            err << "SmartScopesClient::parse_departments(): missing mandatory department attribute '" << field << "'";
            throw LogicException(err.str());
        }
    }

    auto dep = std::make_shared<DepartmentInfo>();
    dep->label = node->get_node("label")->as_string();
    dep->canned_query = node->get_node("canned_query")->as_string();

    if (node->has_node("alternate_label"))
    {
        dep->alternate_label = node->get_node("alternate_label")->as_string();
    }
    if (node->has_node("has_subdepartments"))
    {
        dep->has_subdepartments = node->get_node("has_subdepartments")->as_bool();
    }

    if (node->has_node("subdepartments"))
    {
        auto const subdeps = node->get_node("subdepartments");
        for (int i = 0; i < subdeps->size(); ++i)
        {
            auto child = subdeps->get_node(i);
            try
            {
                dep->subdepartments.push_back(parse_departments(child));
            }
            catch (std::exception const& e)
            {
                // error in one subdepartment is not critical - just ignore it
                std::cerr << "SmartScopesClient::parse_departments(): Error parsing subdepartment of department '" << dep->label << "': " << e.what() << std::endl;
            }
        }
        if(subdeps->size() > 0 && dep->subdepartments.size() == 0)
        {
            std::stringstream err;
            err << "SmartScopesClient::parse_departments(): Failed to parse subdepartments of department '" << dep->label;
            throw LogicException(err.str());
        }
    }
    return dep;
}

Filters SmartScopesClient::parse_filters(JsonNodeInterface::SPtr node)
{
    if (node->type() != JsonNodeInterface::NodeType::Array)
    {
        throw LogicException("SmartScopesClient::parse_filters(): 'filters' attribute must be an array");
    }

    return FilterBaseImpl::deserialize_filters(node->to_variant().get_array());
}

FilterState SmartScopesClient::parse_filter_state(JsonNodeInterface::SPtr node)
{
    if (node->type() != JsonNodeInterface::NodeType::Object)
    {
        throw LogicException("SmartScopesClient::parse_filter_state(): 'filter_state' attribute must be an object");
    }

    return FilterStateImpl::deserialize(node->to_variant().get_dict());
}

void SmartScopesClient::wait_for_preview(uint preview_id)
{
    try
    {
        HttpResponseHandle::SPtr query_result;
        {
            std::lock_guard<std::mutex> lock(query_results_mutex_);

            auto it = query_results_.find(preview_id);
            if (it == query_results_.end())
            {
                throw unity::LogicException("No preview for query " + std::to_string(preview_id) + " is active");
            }

            query_result = it->second;
        }

        query_result->wait();
        query_result->get(); // may throw on error
    }
    catch (std::exception const& e)
    {
        std::cerr << "SmartScopesClient.get_preview_results(): Failed to retrieve preview results for query " << preview_id << std::endl;
        throw;
    }

    std::lock_guard<std::mutex> lock(query_results_mutex_);
    query_results_.erase(preview_id);
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

void SmartScopesClient::cancel_query(uint query_id)
{
    std::lock_guard<std::mutex> lock(query_results_mutex_);

    auto it = query_results_.find(query_id);
    if (it != query_results_.end())
    {
        it->second->cancel();
        query_results_.erase(it);
    }
}

void SmartScopesClient::write_cache(std::string const& scopes_json)
{
    // make cache directory (fails silently if already exists)
    mkdir(c_scopes_cache_dir.c_str(), 0755);

    // open cache for output
    std::ofstream cache_file(c_scopes_cache_dir + c_scopes_cache_filename);

    if (!cache_file.fail())
    {
        cache_file << scopes_json;
        cache_file.close();

        cached_scopes_ = scopes_json;
        have_latest_cache_ = true;
    }
}

std::string SmartScopesClient::read_cache()
{
    if (have_latest_cache_)
    {
        return cached_scopes_;
    }

    // open cache for input
    std::ifstream cache_file(c_scopes_cache_dir + c_scopes_cache_filename);

    if (!cache_file.fail())
    {
        cached_scopes_ = std::string((std::istreambuf_iterator<char>(cache_file)),
                                      std::istreambuf_iterator<char>());

        have_latest_cache_ = true;
    }

    return cached_scopes_;
}

std::string SmartScopesClient::stringify_settings(VariantMap const& settings)
{
    std::ostringstream result_str;
    result_str << "{";

    // Loop through the settings, appending each id:value string to result_str
    bool first_setting = true;
    for (auto const& setting : settings)
    {
        std::ostringstream setting_str;
        setting_str << (first_setting ? "\"" : ",\"") << setting.first << "\":";

        // Determine what the Variant type is and construct the id:value string accordingly
        bool setting_valid = false;
        switch(setting.second.which())
        {
        case Variant::Int:
            setting_str << setting.second.get_int();
            setting_valid = true;
            break;
        case Variant::Bool:
            setting_str << (setting.second.get_bool() ? "true" : "false");
            setting_valid = true;
            break;
        case Variant::String:
            setting_str << "\"" << setting.second.get_string() << "\"";
            setting_valid = true;
            break;
        case Variant::Double:
            setting_str << setting.second.get_double();
            setting_valid = true;
            break;
        default:
            std::cerr << "SmartScopesClient.stringify_settings(): Ignoring unsupported Variant type for settings value: \"" << setting.first << "\"" << std::endl;
        }

        // If we have constructed a valid id:value string, append it to result_str
        if (setting_valid)
        {
            result_str << setting_str.str();
            first_setting = false;
        }
    }
    result_str << "}";

    // If no valid settings were found, return an empty string
    if (first_setting)
    {
        return std::string();
    }
    return result_str.str();
}

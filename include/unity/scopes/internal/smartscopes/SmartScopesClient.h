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

#pragma once

#include <unity/scopes/FilterBase.h>
#include <unity/scopes/FilterState.h>
#include <unity/scopes/internal/smartscopes/HttpClientInterface.h>
#include <unity/scopes/internal/JsonNodeInterface.h>
#include <unity/scopes/internal/Logger.h>
#include <unity/scopes/internal/UniqueID.h>

#include <unity/util/NonCopyable.h>

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <mutex>
#include <tuple>

namespace unity
{

namespace scopes
{

namespace internal
{

namespace smartscopes
{

class SmartScopesClient;

struct RemoteScope
{
    std::string id;
    std::string name;
    std::string description;
    std::string author;
    std::string base_url;
    std::shared_ptr<std::string> icon;          // optional
    std::shared_ptr<std::string> art;           // optional
    std::shared_ptr<VariantMap> appearance;     // optional
    std::shared_ptr<std::string> settings;      // optional
    std::shared_ptr<bool> needs_location_data;  // optional
    bool invisible = false;
    int version;
    std::vector<std::string> tags;              // optional
};

struct SearchCategory
{
    std::string id;
    std::string title;
    std::string icon;
    std::string renderer_template;
};

struct SearchResult
{
    std::string json;
    std::string uri;
    std::map<std::string, JsonNodeInterface::SPtr > other_params;
    std::string category_id;
};

struct DepartmentInfo
{
    std::string label;
    std::string alternate_label;
    std::string canned_query;
    bool has_subdepartments = false;
    std::vector<std::shared_ptr<DepartmentInfo>> subdepartments;
};

struct LocationInfo
{
    bool has_location;
    std::string country_code;
    double latitude;
    double longitude;

    LocationInfo(): has_location(false) {}
};

class SearchHandle
{
public:
    NONCOPYABLE(SearchHandle);
    UNITY_DEFINES_PTRS(SearchHandle);

    ~SearchHandle();

    void wait();
    void cancel_search();

private:
    friend class SmartScopesClient;
    SearchHandle(unsigned int search_id, std::shared_ptr<SmartScopesClient> ssc);

private:
    unsigned int search_id_;
    std::shared_ptr<SmartScopesClient> ssc_;
};

class PreviewHandle
{
public:
    NONCOPYABLE(PreviewHandle);
    UNITY_DEFINES_PTRS(PreviewHandle);

    ~PreviewHandle();

    using Columns = std::vector<std::vector<std::vector<std::string>>>;
    using Widgets = std::vector<std::string>;

    void wait();
    void cancel_preview();

private:
    friend class SmartScopesClient;
    PreviewHandle(unsigned int preview_id, std::shared_ptr<SmartScopesClient> ssc);

private:
    unsigned int preview_id_;
    std::shared_ptr<SmartScopesClient> ssc_;
};

struct SearchReplyHandler
{
    std::function<void(SearchResult const&)> result_handler;
    std::function<void(std::shared_ptr<SearchCategory> const&)> category_handler;
    std::function<void(std::shared_ptr<DepartmentInfo> const&)> departments_handler;
    std::function<void(Filters const&)> filters_handler;
    std::function<void(FilterState const&)> filter_state_handler;
};

struct PreviewReplyHandler
{
    std::function<void(std::string const&)> widget_handler;
    std::function<void(PreviewHandle::Columns const&)> columns_handler;
};

class SmartScopesClient : public std::enable_shared_from_this<SmartScopesClient>
{
public:
    NONCOPYABLE(SmartScopesClient);
    UNITY_DEFINES_PTRS(SmartScopesClient);

    SmartScopesClient(HttpClientInterface::SPtr http_client,
                      JsonNodeInterface::SPtr json_node,
                      boost::log::sources::severity_channel_logger_mt<>& logger_,
                      std::string const& url = ""); // detect url

    virtual ~SmartScopesClient();

    void reset_url(std::string const& url = "");
    std::string url();

    bool get_remote_scopes(std::vector<RemoteScope>& scopes, std::string const& locale = "", bool caching_enabled = true);

    SearchHandle::UPtr search(SearchReplyHandler const& handler,
                              std::string const& base_url,
                              std::string const& query,
                              std::string const& department_id,
                              std::string const& session_id,
                              int query_id,
                              std::string const& platform,
                              VariantMap const& settings = VariantMap(),
                              VariantMap const& filter_state = VariantMap(),
                              std::string const& locale = "",
                              LocationInfo const& location = LocationInfo(),
                              std::string const& user_agent_hdr = "",
                              const unsigned int limit = 0);

    PreviewHandle::UPtr preview(PreviewReplyHandler const& handler,
                                std::string const& base_url,
                                std::string const& result,
                                std::string const& session_id,
                                std::string const& platform,
                                const unsigned int widgets_api_version,
                                VariantMap const& settings = VariantMap(),
                                std::string const& locale = "",
                                std::string const& country = "",
                                std::string const& user_agent_hdr = "");

    boost::log::sources::severity_channel_logger_mt<>& logger() const;

private:
    friend class SearchHandle;
    friend class PreviewHandle;

    void wait_for_search(unsigned int search_id);
    void wait_for_preview(unsigned int preview_id);
    std::shared_ptr<DepartmentInfo> parse_departments(JsonNodeInterface::SPtr node);
    Filters parse_filters(JsonNodeInterface::SPtr node);
    FilterState parse_filter_state(JsonNodeInterface::SPtr node);
    void parse_line(std::string const& json, SearchReplyHandler const& handler);
    void parse_line(std::string const& json, PreviewReplyHandler const& handler);

    std::vector<std::string> extract_json_stream(std::string const& json_stream);

    void cancel_query(unsigned int query_id);

    void write_cache(std::string const& scopes_json);
    std::string read_cache();

    std::string stringify_settings(VariantMap const& settings);

private:
    HttpClientInterface::SPtr http_client_;
    JsonNodeInterface::SPtr json_node_;
    boost::log::sources::severity_channel_logger_mt<>& logger_;
    std::string url_;

    std::map<unsigned int, HttpResponseHandle::SPtr> query_results_;

    std::mutex json_node_mutex_;
    std::mutex query_results_mutex_;

    std::string cached_scopes_;
    bool have_latest_cache_;

    unsigned int query_counter_;
};

}  // namespace smartscopes

}  // namespace internal

}  // namespace scopes

}  // namespace unity

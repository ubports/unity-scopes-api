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

#ifndef UNITY_SCOPES_INTERNAL_SMARTSCOPES_SMARTSCOPESCLIENT_H
#define UNITY_SCOPES_INTERNAL_SMARTSCOPES_SMARTSCOPESCLIENT_H

#include <unity/scopes/internal/smartscopes/HttpClientInterface.h>
#include <unity/scopes/internal/JsonNodeInterface.h>

#include <unity/util/NonCopyable.h>

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <mutex>

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
    std::string base_url;
    std::string icon;
    bool invisible = false;
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
    std::string title;
    std::string art;
    std::string dnd_uri;
    std::map<std::string, JsonNodeInterface::SPtr > other_params;
    std::shared_ptr<SearchCategory> category;
};

class SearchHandle
{
public:
    NONCOPYABLE(SearchHandle);
    UNITY_DEFINES_PTRS(SearchHandle);

    ~SearchHandle();

    std::vector<SearchResult> get_search_results();
    void cancel_search();

private:
    friend class SmartScopesClient;
    SearchHandle(std::string const& session_id, std::shared_ptr<SmartScopesClient> ssc);

private:
    std::string session_id_;
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

    std::pair<Columns, Widgets> get_preview_results();
    void cancel_preview();

private:
    friend class SmartScopesClient;
    PreviewHandle(std::string const& session_id, std::shared_ptr<SmartScopesClient> ssc);

private:
    std::string session_id_;
    std::shared_ptr<SmartScopesClient> ssc_;
};

class SmartScopesClient : public std::enable_shared_from_this<SmartScopesClient>
{
public:
    NONCOPYABLE(SmartScopesClient);
    UNITY_DEFINES_PTRS(SmartScopesClient);

    SmartScopesClient(HttpClientInterface::SPtr http_client,
                      JsonNodeInterface::SPtr json_node,
                      std::string const& url = "" /*detect url*/,
                      uint port = 80);

    virtual ~SmartScopesClient();

    std::vector<RemoteScope> get_remote_scopes(std::string const& locale = "");

    SearchHandle::UPtr search(std::string const& base_url,
                              std::string const& query,
                              std::string const& session_id,
                              uint query_id,
                              std::string const& platform,
                              std::string const& locale = "",
                              std::string const& country = "",
                              const uint limit = 0);

    PreviewHandle::UPtr preview(std::string const& base_url,
                                std::string const& result,
                                std::string const& session_id,
                                std::string const& platform,
                                const uint widgets_api_version,
                                std::string const& locale = "",
                                std::string const& country = "");

private:
    friend class SearchHandle;
    friend class PreviewHandle;

    std::vector<SearchResult> get_search_results(std::string const& session_id);
    std::pair<PreviewHandle::Columns, PreviewHandle::Widgets> get_preview_results(std::string const& session_id);

    std::vector<std::string> extract_json_stream(std::string const& json_stream);

    void cancel_search(std::string const& session_id);
    void cancel_preview(std::string const& session_id);

private:
    HttpClientInterface::SPtr http_client_;
    JsonNodeInterface::SPtr json_node_;

    std::string url_;
    uint port_;

    std::map<std::string, HttpResponseHandle::SPtr> search_results_;
    std::map<std::string, HttpResponseHandle::SPtr> preview_results_;

    std::mutex json_node_mutex_;
    std::mutex search_results_mutex_;
    std::mutex preview_results_mutex_;
};

}  // namespace smartscopes

}  // namespace internal

}  // namespace scopes

}  // namespace unity

#endif  // UNITY_SCOPES_INTERNAL_SMARTSCOPES_SMARTSCOPESCLIENT_H

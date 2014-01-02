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
#include <scopes/internal/smartscopes/JsonNodeInterface.h>

#include <unity/util/NonCopyable.h>

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <mutex>

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

class SmartScopesClient;

struct RemoteScope
{
    std::string name;
    std::string search_url;
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
    std::string uri;
    std::string title;
    std::string art;
    std::string dnd_uri;
    std::shared_ptr<SearchCategory> category;
};

class SearchHandle
{
public:
    NONCOPYABLE(SearchHandle);
    UNITY_DEFINES_PTRS(SearchHandle);

    ~SearchHandle();

    std::vector<SearchResult> get_search_results();

private:
    friend class SmartScopesClient;
    SearchHandle(std::string const& session_id, std::shared_ptr<SmartScopesClient> ssc);

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

    std::vector<RemoteScope> get_remote_scopes();

    SearchHandle::UPtr search(std::string const& search_url, std::string const& query,
                              std::string const& session_id, uint query_id, std::string const& platform,
                              std::string const& locale = "", std::string const& country = "",
                              std::string const& latitude = "", std::string const& longitude = "",
                              const uint limit = 0);

private:
    friend class SearchHandle;
    std::vector<SearchResult> get_search_results(std::string const& session_id);
    void cancel_search(std::string const& session_id);

    std::vector<std::string> extract_json_stream(std::string const& json_stream);

private:
    HttpClientInterface::SPtr http_client_;
    JsonNodeInterface::SPtr json_node_;

    std::string url_;
    uint port_;

    std::map<std::string, HttpResponseHandle::SPtr> search_results_;

    std::mutex json_node_mutex_;
    std::mutex search_results_mutex_;
};

} // namespace smartscopes

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

#endif // UNITY_API_SCOPES_INTERNAL_SMARTSCOPES_SMARTSCOPESCLIENT_H

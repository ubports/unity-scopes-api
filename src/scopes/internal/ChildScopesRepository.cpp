/*
 * Copyright (C) 2014 Canonical Ltd
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

#include <unity/scopes/internal/ChildScopesRepository.h>
#include <unity/scopes/internal/JsonCppNode.h>

#include <cassert>
#include <fstream>
#include <set>
#include <sstream>

using namespace unity::scopes;
using namespace unity::scopes::internal;

ChildScopesRepository::ChildScopesRepository(std::string const& repo_file_path, Logger& logger)
    : repo_file_path_(repo_file_path)
    , logger_(logger)
    , have_latest_cache_(false)
{
}

ChildScopeList ChildScopesRepository::child_scopes(ChildScopeList const& child_scopes_defaulted)
{
    ChildScopeList child_scopes;

    std::lock_guard<std::mutex> lock(mutex_);

    // Read child scope enabled states from our repo
    auto child_enabled_map = read_repo();

    // If we find a child scope in the repo, replace its enabled state with the one found in the repo
    for (auto const& child_scope : child_scopes_defaulted)
    {
        if (child_enabled_map.find(child_scope.id) != child_enabled_map.end())
        {
            ChildScope updated_child_scope = child_scope;
            updated_child_scope.enabled = child_enabled_map.at(child_scope.id);
            child_scopes.push_back(updated_child_scope);
        }
        else
        {
            // If we don't find a child scope in the repo, simply add it in default state
            child_scopes.push_back(child_scope);
        }
    }

    // write the new ordered list to file
    write_repo(child_scopes);
    return child_scopes;
}

bool ChildScopesRepository::set_child_scopes(ChildScopeList const& child_scopes)
{
    // simply write child_scopes_ordered to file
    std::lock_guard<std::mutex> lock(mutex_);
    return write_repo(child_scopes);
}

bool ChildScopesRepository::write_repo(ChildScopeList const& child_scopes_list)
{
    assert(!mutex_.try_lock());

    // open repository for output
    std::ofstream repo_file(repo_file_path_);
    if (repo_file.fail())
    {
        logger_() << "ChildScopesRepository::write_repo(): Failed to open file: \"" << repo_file_path_ << "\"";
        return false;
    }

    repo_file << list_to_json(child_scopes_list);
    repo_file.close();

    have_latest_cache_ = false;
    return true;
}

ChildScopeEnabledMap ChildScopesRepository::read_repo()
{
    // if we already have the latest cache of the repo, simply return it
    if (have_latest_cache_)
    {
        return cached_repo_;
    }

    // open repository for input
    std::ifstream repo_file(repo_file_path_);
    if (repo_file.fail())
    {
        logger_(LoggerSeverity::Info) << "ChildScopesRepository::read_repo(): "
                                      << "Failed to open file: \"" << repo_file_path_ << "\"";
        return ChildScopeEnabledMap();
    }

    cached_repo_ = json_to_list(std::string((std::istreambuf_iterator<char>(repo_file)),
                                            std::istreambuf_iterator<char>()));
    repo_file.close();

    have_latest_cache_ = true;
    return cached_repo_;
}

std::string ChildScopesRepository::list_to_json(ChildScopeList const& child_scopes_list)
{
    std::ostringstream child_scopes_json;
    child_scopes_json << "[";
    for (auto const& child_scope : child_scopes_list)
    {
        child_scopes_json << (child_scopes_json.tellp() == 1 ? "{" : ",{");
        child_scopes_json << "\"id\":\"" << child_scope.id << "\"";
        child_scopes_json << ",";
        child_scopes_json << "\"enabled\":" << (child_scope.enabled ? "true" : "false");
        child_scopes_json << "}";
    }
    child_scopes_json << "]";
    return child_scopes_json.str();
}

ChildScopeEnabledMap ChildScopesRepository::json_to_list(std::string const& child_scopes_json)
{
    JsonCppNode json_node;
    try
    {
        json_node.read_json(child_scopes_json);
    }
    catch (std::exception const& e)
    {
        logger_() << "ChildScopesRepository::json_to_list(): Exception thrown while reading json string: " << e.what();
        return ChildScopeEnabledMap();
    }
    if (json_node.type() != JsonCppNode::Array)
    {
        logger_() << "ChildScopesRepository::json_to_list(): Root node of json string is not an array:"
                  << std::endl << child_scopes_json;
        return ChildScopeEnabledMap();
    }

    ChildScopeEnabledMap return_map;
    for (int i = 0; i < json_node.size(); ++i)
    {
        auto child_node = json_node.get_node(i);

        if (!child_node->has_node("id") ||
            !child_node->has_node("enabled"))
        {
            logger_() << "ChildScopesRepository::json_to_list(): Child node is missing a required field. "
                     << "Skipping child node:" << std::endl << child_node->to_json_string();
            continue;
        }
        auto id_node = child_node->get_node("id");
        auto enabled_node = child_node->get_node("enabled");

        if (id_node->type() != JsonCppNode::String ||
            enabled_node->type() != JsonCppNode::Bool)
        {
            logger_() << "ChildScopesRepository::json_to_list(): "
                      << "Child node contains an invalid value type. Skipping child node:"
                      << std::endl << child_node->to_json_string();
            continue;
        }

        // If an aggregator aggregates the same child scope more than once (say, for different keywords),
        // we count the enabled state of its first appearance as the overall enabled state of that child.
        if (return_map.find(id_node->as_string()) == return_map.end())
        {
            return_map.insert(make_pair(id_node->as_string(), enabled_node->as_bool()));
        }
    }
    return return_map;
}

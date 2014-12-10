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

#include <fstream>
#include <sstream>

using namespace unity::scopes;
using namespace unity::scopes::internal;

ChildScopesRepository::ChildScopesRepository(std::string const& repo_file_path)
    : repo_file_path_(repo_file_path)
    , have_latest_cache_(false)
{
}

ChildScopeList ChildScopesRepository::child_scopes_ordered(ChildScopeList const& child_scopes_unordered) const
{
    ///!
}

void ChildScopesRepository::set_child_scopes_ordered(ChildScopeList const& child_scopes_ordered)
{
    // simply write child_scopes_ordered to file
    write_repo(list_to_json(child_scopes_ordered));
}

void ChildScopesRepository::write_repo(std::string const& child_scopes_json)
{
    // open repository for output
    std::ofstream repo_file(repo_file_path_);

    if (!repo_file.fail())
    {
        repo_file << child_scopes_json;
        repo_file.close();

        cached_repo_ = json_to_list(child_scopes_json);
        have_latest_cache_ = true;
    }
}

ChildScopeList ChildScopesRepository::read_repo()
{
    // if we already have the latest cache of the repo, simply return it
    if (have_latest_cache_)
    {
        return cached_repo_;
    }

    // open repository for input
    std::ifstream repo_file(repo_file_path_);

    if (!repo_file.fail())
    {
        cached_repo_ = json_to_list(std::string((std::istreambuf_iterator<char>(repo_file)),
                                                std::istreambuf_iterator<char>()));
        repo_file.close();

        have_latest_cache_ = true;
    }

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

ChildScopeList ChildScopesRepository::json_to_list(std::string const& child_scopes_json)
{
    ChildScopeList return_list;

    JsonCppNode json_node(child_scopes_json);
    if (json_node.type() != JsonCppNode::Array)
    {
        throw;
    }

    for (int i = 0; i < json_node.size(); ++i)
    {
        auto child_node = json_node.get_node(i);

        if (!child_node->has_node("id") || !child_node->has_node("enabled"))
        {
            continue;
        }
        auto id_node = json_node.get_node("id");
        auto enabled_node = json_node.get_node("enabled");

        if (id_node->type() != JsonCppNode::String || enabled_node->type() != JsonCppNode::Bool)
        {
            continue;
        }
        return_list.push_back( ChildScope{id_node->as_string(), enabled_node->as_bool()} );
    }
    return return_list;
}

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
#include <set>
#include <sstream>

using namespace unity::scopes;
using namespace unity::scopes::internal;

ChildScopesRepository::ChildScopesRepository(std::string const& repo_file_path,
                                             boost::log::sources::severity_channel_logger_mt<>& logger)
    : repo_file_path_(repo_file_path)
    , logger_(logger)
    , have_latest_cache_(false)
{
}

ChildScopeList ChildScopesRepository::child_scopes(ChildScopeList const& child_scopes_unordered)
{
    std::lock_guard<std::mutex> lock(mutex_);

    // unordered_set and ordered_set will act as masks for child_scopes_unordered and
    // child_scopes_ordered respectively when creating our resultant child scope list
    std::map<std::string, int> unordered_set;
    std::map<std::string, int> ordered_set;

    // fill unordered_set
    int i = 0;
    for (auto const& child : child_scopes_unordered)
    {
        unordered_set.insert(std::make_pair(child.id, i++));
    }

    // fill ordered_set
    ChildScopeList child_scopes_ordered = read_repo();
    for (auto const& child : child_scopes_ordered)
    {
        // scopes in child_scopes_ordered that are also in child_scopes_unordered should be removed
        // from unordered_set (to avoid duplicates in our resultant child scope list)
        auto it = unordered_set.find(child.id);
        if (it != unordered_set.end())
        {
            // only the scopes from child_scopes_ordered that appear in child_scopes_unordered
            // should be added to ordered_set (a scope not found in child_scopes_unordered was
            // probably uninstalled since the repo was last written)

            // store index to item in child_scopes_unordered ->
            ordered_set.insert(std::make_pair(child.id, it->second));
            unordered_set.erase(it);
        }
    }

    // has anything changed since the repo was last written?
    if (child_scopes_ordered.size() == ordered_set.size() && // if no scopes have been removed
        unordered_set.size() == 0)                           // and no scopes have been added
    {
        return child_scopes_ordered; // return the repo list as is
    }

    // now create a new list by first adding child_scopes_ordered then child_scopes_unordered,
    // using each set as a mask to determine whether or not a scope should be included
    ChildScopeList new_child_scopes_ordered;
    {
        // first add child_scopes_ordered to new_child_scopes_ordered
        // (known scopes from repo)
        if (child_scopes_ordered.size() == ordered_set.size())
        {
            new_child_scopes_ordered = child_scopes_ordered;
        }
        else if (!ordered_set.empty())
        {
            // use ordered_set as a mask
            for (auto& child : child_scopes_ordered)
            {
                auto it = ordered_set.find(child.id);
                if (it != ordered_set.end())
                {
                    // -> always use latest keywords list from child_scopes_unordered
                    child.keywords = child_scopes_unordered[it->second].keywords;
                    new_child_scopes_ordered.push_back(child);
                }
            }
        }

        // then append whats left in child_scopes_unordered to new_child_scopes_ordered
        // (newly installed scopes)
        if (child_scopes_unordered.size() == unordered_set.size())
        {
            new_child_scopes_ordered.insert(new_child_scopes_ordered.end(),
                                            child_scopes_unordered.begin(),
                                            child_scopes_unordered.end());
        }
        else if (!unordered_set.empty())
        {
            // use unordered_set as a mask
            for (auto const& child : child_scopes_unordered)
            {
                if (unordered_set.find(child.id) != unordered_set.end())
                {
                    new_child_scopes_ordered.push_back(child);
                }
            }
        }
    }

    // write the new ordered list to file
    write_repo(new_child_scopes_ordered);
    return new_child_scopes_ordered;
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
        BOOST_LOG_SEV(logger_, Logger::Error) << "ChildScopesRepository::write_repo(): "
                                              << "Failed to open file: \"" << repo_file_path_
                                              << "\"";
        return false;
    }

    repo_file << list_to_json(child_scopes_list);
    repo_file.close();

    cached_repo_ = child_scopes_list;
    have_latest_cache_ = true;
    return true;
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
    if (repo_file.fail())
    {
        BOOST_LOG_SEV(logger_, Logger::Info) << "ChildScopesRepository::read_repo(): "
                                             << "Failed to open file: \"" << repo_file_path_
                                             << "\"";
        return ChildScopeList();
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

ChildScopeList ChildScopesRepository::json_to_list(std::string const& child_scopes_json)
{
    JsonCppNode json_node;
    try
    {
        json_node.read_json(child_scopes_json);
    }
    catch (std::exception const& e)
    {
        BOOST_LOG_SEV(logger_, Logger::Error) << "ChildScopesRepository::json_to_list(): "
                                              << "Exception thrown while reading json string: "
                                              << e.what();
        return ChildScopeList();
    }
    if (json_node.type() != JsonCppNode::Array)
    {
        BOOST_LOG_SEV(logger_, Logger::Error) << "ChildScopesRepository::json_to_list(): "
                                              << "Root node of json string is not an array:"
                                              << std::endl << child_scopes_json;
        return ChildScopeList();
    }

    ChildScopeList return_list;
    for (int i = 0; i < json_node.size(); ++i)
    {
        auto child_node = json_node.get_node(i);

        if (!child_node->has_node("id") ||
            !child_node->has_node("enabled"))
        {
            BOOST_LOG_SEV(logger_, Logger::Error) << "ChildScopesRepository::json_to_list(): "
                                                  << "Child node is missing a required field. "
                                                  << "Skipping child node:" << std::endl
                                                  << child_node->to_json_string();
            continue;
        }
        auto id_node = child_node->get_node("id");
        auto enabled_node = child_node->get_node("enabled");

        if (id_node->type() != JsonCppNode::String ||
            enabled_node->type() != JsonCppNode::Bool)
        {
            BOOST_LOG_SEV(logger_, Logger::Error) << "ChildScopesRepository::json_to_list(): "
                                                  << "Child node contains an invalid value type. "
                                                  << "Skipping child node:" << std::endl
                                                  << child_node->to_json_string();
            continue;
        }
        return_list.push_back( ChildScope{id_node->as_string(),
                                          enabled_node->as_bool()} );
    }
    return return_list;
}

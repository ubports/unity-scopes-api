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

#pragma once

#include <unity/scopes/internal/ChildScopesRepository.h>

#include <fstream>
#include <sstream>

using namespace unity::scopes;
using namespace unity::scopes::internal;

ChildScopesRepository::ChildScopesRepository(std::string const& repo_file_path)
    : repo_file_path_(repo_file_path)
    , cached_repo_(new JsonCppNode)
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
    std::ostringstream child_scopes_json;
    child_scopes_json << "[";
    for (auto const& child_scope : child_scopes_ordered)
    {
        child_scopes_json << (child_scopes_json.tellp() == 1 ? "{" : ",{");
        child_scopes_json << "\"id\":\"" << child_scope.id << "\"";
        child_scopes_json << ",";
        child_scopes_json << "\"enabled\":" << (child_scope.enabled ? "true" : "false");
        child_scopes_json << "}";
    }
    child_scopes_json << "]";
    write_repo(child_scopes_json.str());
}

void ChildScopesRepository::write_repo(std::string const& child_scopes_json)
{
    // open repository for output
    std::ofstream repo_file(repo_file_path_);

    if (!repo_file.fail())
    {
        repo_file << child_scopes_json;
        repo_file.close();

        cached_repo_->read_json(child_scopes_json);
        have_latest_cache_ = true;
    }
}

JsonCppNode::SPtr ChildScopesRepository::read_repo()
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
        cached_repo_->read_json(std::string((std::istreambuf_iterator<char>(repo_file)),
                                            std::istreambuf_iterator<char>()));
        repo_file.close();

        have_latest_cache_ = true;
    }

    return cached_repo_;
}

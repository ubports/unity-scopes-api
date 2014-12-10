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

#include <unity/scopes/ChildScope.h>
#include <unity/scopes/internal/Logger.h>
#include <unity/util/NonCopyable.h>

#include <list>
#include <mutex>
#include <string>

namespace unity
{

namespace scopes
{

namespace internal
{

class ChildScopesRepository
{
public:
    NONCOPYABLE(ChildScopesRepository);

    ChildScopesRepository(std::string const& repo_file_path,
                          boost::log::sources::severity_channel_logger_mt<>& logger);

    ChildScopeList child_scopes_ordered(ChildScopeList const& child_scopes_unordered);
    void set_child_scopes_ordered(ChildScopeList const& child_scopes_ordered);

private:
    void write_repo(std::string const& child_scopes_json);
    ChildScopeList read_repo();

    std::string list_to_json(ChildScopeList const& child_scopes_list);
    ChildScopeList json_to_list(std::string const& child_scopes_json);

    std::string const repo_file_path_;
    boost::log::sources::severity_channel_logger_mt<>& logger_;

    mutable std::mutex mutex_;
    ChildScopeList cached_repo_;
    bool have_latest_cache_;
};

} // namespace internal

} // namespace scopes

} // namespace unity

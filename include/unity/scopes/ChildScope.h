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

#include <set>
#include <string>
#include <vector>

namespace unity
{

namespace scopes
{

struct ChildScope;

/**
 \brief A list of child scopes.
*/

typedef std::vector<ChildScope> ChildScopeList;

/**
 \brief A container for details about an aggregator's child scope.
*/

struct ChildScope
{
    ChildScope(std::string const& id,
               bool enabled,
               std::set<std::string> keywords = {})
        : id(id)
        , enabled(enabled)
        , keywords(keywords) {}

    std::string id;                 ///< The scope id of this child scope.
    bool enabled;                   ///< Holds whether this child scope is aggregated.
    std::set<std::string> keywords; ///< A list of keywords used to aggregate this scope.
};

} // namespace scopes

} // namespace unity

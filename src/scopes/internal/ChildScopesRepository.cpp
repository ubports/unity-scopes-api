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

using namespace unity::scopes;
using namespace unity::scopes::internal;

ChildScopesRepository::ChildScopesRepository(std::string const& repo_file_path)
{

}

ChildScopeList ChildScopesRepository::child_scopes_ordered(ChildScopeList const& child_scopes_unordered) const
{

}

void ChildScopesRepository::set_child_scopes_ordered(ChildScopeList const& child_scopes_ordered)
{

}

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
 * Authored by: Pete Woods <pete.woods@canonical.com>
 */

#include <unity/scopes/ScopeBase.h>
#include <unity/scopes/internal/ScopeBaseImpl.h>
#include <unity/scopes/testing/TypedScopeFixture.h>

using namespace unity::scopes::testing;

/// @cond
void TypedScopeFixtureHelper::set_registry(std::shared_ptr<unity::scopes::ScopeBase> const& scope, RegistryProxy const& r)
{
    scope->p->set_registry(r);
}

void TypedScopeFixtureHelper::set_scope_directory(std::shared_ptr<unity::scopes::ScopeBase> const& scope, std::string const& path)
{
    scope->p->set_scope_directory(path);
}
/// @endcond

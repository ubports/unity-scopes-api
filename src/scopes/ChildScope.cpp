/*
 * Copyright (C) 2015 Canonical Ltd
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

#include <unity/scopes/ChildScope.h>
#include <unity/scopes/internal/ScopeMetadataImpl.h>

namespace unity
{

namespace scopes
{

ChildScope::ChildScope(std::string const& id,
                       bool enabled,
                       std::set<std::string> keywords)
    : metadata(internal::ScopeMetadataImpl::create(nullptr)) ///!
    , id(id)
    , enabled(enabled)
    , keywords(keywords) {}

} // namespace scopes

} // namespace unity

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
 * Authored by: Pawel Stolowski <pawel.stolowski@canonical.com>
 */

#include <unity/scopes/internal/ScopeBaseImpl.h>

using namespace std;

namespace unity
{

namespace scopes
{

namespace internal
{

void ScopeBaseImpl::set_scope_directory(std::string const& path)
{
    lock_guard<mutex> lock(mutex_);
    scope_directory_ = path;
}

std::string ScopeBaseImpl::scope_directory() const
{
    lock_guard<mutex> lock(mutex_);
    return scope_directory_;
}

void ScopeBaseImpl::set_cache_directory(std::string const& path)
{
    lock_guard<mutex> lock(mutex_);
    cache_directory_ = path;
}

std::string ScopeBaseImpl::cache_directory() const
{
    lock_guard<mutex> lock(mutex_);
    return cache_directory_;
}

void ScopeBaseImpl::set_registry(RegistryProxy const& registry)
{
    lock_guard<mutex> lock(mutex_);
    registry_ = registry;
}

RegistryProxy ScopeBaseImpl::registry() const
{
    lock_guard<mutex> lock(mutex_);
    return registry_;
}

} // namespace internal

} // namespace scopes

} // namespace unity

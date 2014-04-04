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

#ifndef UNITY_SCOPES_INTERNAL_SCOPEBASEIMPL_H
#define UNITY_SCOPES_INTERNAL_SCOPEBASEIMPL_H

#include <string>

namespace unity
{

namespace scopes
{

namespace internal
{

class ScopeBaseImpl final
{
public:
    ScopeBaseImpl() = default;
    ~ScopeBaseImpl() = default;
    void set_scope_directory(std::string const &path);
    std::string scope_directory() const;

private:
    std::string scope_directory_;
};

} // namespace internal

} // namespace scopes

} // namespace unity

#endif

/*
 * Copyright (C) 2013 Canonical Ltd
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
 * Authored by: Michi Henning <michi.henning@canonical.com>
 */

#ifndef UNITY_API_SCOPES_INTERNAL_MWREGISTRY_H
#define UNITY_API_SCOPES_INTERNAL_MWREGISTRY_H

#include <scopes/internal/MWObjectProxy.h>
#include <scopes/Registry.h>
#include <scopes/ScopeMetadata.h>

namespace unity
{

namespace scopes
{

namespace internal
{

class MWRegistry : public virtual MWObjectProxy
{
public:
    // Remote operation implementation
    virtual ScopeMetadata get_metadata(std::string const& scope_name) = 0;
    virtual MetadataMap list() = 0;

    virtual ~MWRegistry() noexcept;

protected:
    MWRegistry(MiddlewareBase* mw_base);
};

} // namespace internal

} // namespace scopes

} // namespace unity

#endif

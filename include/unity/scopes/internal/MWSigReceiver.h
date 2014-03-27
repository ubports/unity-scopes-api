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
 * Authored by: Marcus Tomlinson <marcus.tomlinson@canonical.com>
 */

#ifndef UNITY_SCOPES_INTERNAL_MWSIGRECEIVER_H
#define UNITY_SCOPES_INTERNAL_MWSIGRECEIVER_H

#include <unity/scopes/internal/MWObjectProxy.h>
#include <unity/scopes/Registry.h>
#include <unity/scopes/ScopeMetadata.h>

namespace unity
{

namespace scopes
{

namespace internal
{

class MWSigReceiver : public virtual MWObjectProxy
{
public:
    // Remote operations
    virtual ScopeMetadata get_metadata(std::string const& scope_id) = 0;
    virtual MetadataMap list() = 0;
    virtual ScopeProxy locate(std::string const& scope_id) = 0;

    virtual ~MWSigReceiver();

protected:
    MWSigReceiver(MiddlewareBase* mw_base);
};

} // namespace internal

} // namespace scopes

} // namespace unity

#endif // UNITY_SCOPES_INTERNAL_MWSIGRECEIVER_H

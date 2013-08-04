/*
 * Copyright (C) 2013 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Michi Henning <michi.henning@canonical.com>
 */

#ifndef UNITY_API_SCOPES_INTERNAL_ICEMIDDLEWARE_REGISTRYI_H
#define UNITY_API_SCOPES_INTERNAL_ICEMIDDLEWARE_REGISTRYI_H

#include <slice/unity/api/scopes/internal/ice_middleware/Registry.h>
#include <unity/api/scopes/internal/RegistryObject.h>

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

namespace ice_middleware
{

// Server-side implementation of a Registry object, which maps scope names to scope proxies.
// It provides remote lookup and listing of the map contents, and local methods to manipulate the map.
//
// This class is thread-safe: differnt threads can concurrently update the map while lookup operations
// are in progress.

class RegistryI : public middleware::Registry
{
public:
    RegistryI(RegistryObject::SPtr const& ro);
    virtual ~RegistryI() noexcept;

    // Slice operations
    virtual middleware::ScopePrx find(std::string const& name, Ice::Current const&) override;
    virtual middleware::ScopeDict list(Ice::Current const&) override;

private:
    RegistryObject::SPtr ro_;
};

typedef IceUtil::Handle<RegistryI> RegistryIPtr;

} // namespace ice_middleware

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

#endif

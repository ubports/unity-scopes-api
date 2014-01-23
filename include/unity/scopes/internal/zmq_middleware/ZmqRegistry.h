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

#ifndef UNITY_SCOPES_INTERNAL_ZMQMIDDLEWARE_ZMQREGISTRY_H
#define UNITY_SCOPES_INTERNAL_ZMQMIDDLEWARE_ZMQREGISTRY_H

#include <unity/scopes/internal/zmq_middleware/ZmqObjectProxy.h>
#include <unity/scopes/internal/zmq_middleware/ZmqRegistryProxyFwd.h>
#include <unity/scopes/internal/MWRegistry.h>

namespace unity
{

namespace scopes
{

namespace internal
{

namespace zmq_middleware
{

// Client-side registry proxy for Zmq. The implementation forwards the invocations via Zmq,
// and translates the parameters and return value between the Zmq types and the public types.

class ZmqRegistry : public virtual ZmqObjectProxy, public virtual MWRegistry
{
public:
    ZmqRegistry(ZmqMiddleware* mw_base,
                std::string const& endpoint,
                std::string const& identity,
                std::string const& category);
    virtual ~ZmqRegistry() noexcept;

    // Remote operations.
    virtual ScopeMetadata get_metadata(std::string const& scope_name) override;
    virtual MetadataMap list() override;
    virtual ScopeProxy locate(std::string const& scope_name) override;
};

} // namespace zmq_middleware

} // namespace internal

} // namespace scopes

} // namespace unity

#endif

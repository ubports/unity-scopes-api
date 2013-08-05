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

#ifndef UNITY_API_SCOPES_INTERNAL_ICEMIDDLEWARE_ICEMIDDLEWARE_H
#define UNITY_API_SCOPES_INTERNAL_ICEMIDDLEWARE_ICEMIDDLEWARE_H

#include <unity/api/scopes/internal/ice_middleware/IceObjectProxy.h>
#include <unity/api/scopes/internal/ice_middleware/IceReplyProxy.h>
#include <unity/api/scopes/internal/MiddlewareBase.h>

#include <Ice/CommunicatorF.h>
#include <Ice/ObjectAdapterF.h>

namespace unity
{

namespace api
{

namespace scopes
{

class ScopeBase;

namespace internal
{

namespace ice_middleware
{

class IceMiddleware : public MiddlewareBase
{
public:
    IceMiddleware(std::string const& server_name, std::string const& configfile);
    virtual ~IceMiddleware() noexcept;

    virtual void start() override;
    virtual void stop() override;
    virtual void wait_for_shutdown() override;

    virtual MWRegistryProxy::SPtr create_registry_proxy(std::string const& identity, std::string const& endpoint) override;
    virtual MWScopeProxy::SPtr create_scope_proxy(std::string const& identity, std::string const& endpoint) override;

    virtual MWRegistryProxy::SPtr add_registry_object(std::string const& identity, RegistryObject::SPtr const& registry) override;
    virtual MWScopeProxy::SPtr add_scope_object(std::string const& identity, ScopeObject::SPtr const& scope) override;
    virtual MWReplyProxy::SPtr add_reply_object(ReplyObject::SPtr const& reply) override;

    virtual void remove_object(std::string const& identity);

private:
    Ice::ObjectPrx safe_add(Ice::ObjectPtr const& obj, std::string const& identity = "");

    std::string server_name_;
    std::string configfile_;
    Ice::CommunicatorPtr ic_;
    Ice::ObjectAdapterPtr adapter_;
    std::mutex mutex_;              // Protects adapter_
};

} // namespace ice_middleware

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

#endif

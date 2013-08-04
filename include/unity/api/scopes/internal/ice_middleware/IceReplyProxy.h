/*
 * Copyright (C) 2013 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the Lesser GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * Lesser GNU General Public License for more details.
 *
 * You should have received a copy of the Lesser GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Michi Henning <michi.henning@canonical.com>
 */

#ifndef UNITY_API_SCOPES_INTERNAL_ICEMIDDLEWARE_ICEREPLYPROXY_H
#define UNITY_API_SCOPES_INTERNAL_ICEMIDDLEWARE_ICEREPLYPROXY_H

#include <slice/unity/api/scopes/internal/ice_middleware/Reply.h>
#include <unity/api/scopes/internal/MWReplyProxy.h>

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

class IceReplyProxy : public MWReplyProxy
{
public:
    UNITY_DEFINES_PTRS(IceReplyProxy);

    IceReplyProxy(MiddlewareBase* mw_base, Ice::ObjectPrx const& p) noexcept;
    ~IceReplyProxy() noexcept;

    virtual void send(std::string const& result) override;
    virtual void finished() override;

    middleware::ReplyPrx proxy() const noexcept;

private:
    middleware::ReplyPrx proxy_;
};

} // namespace ice_middleware

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

#endif

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

#ifndef UNITY_API_SCOPES_INTERNAL_ICEMIDDLEWARE_ICEREPLY_H
#define UNITY_API_SCOPES_INTERNAL_ICEMIDDLEWARE_ICEREPLY_H

#include <unity/api/scopes/internal/ice_middleware/slice/Reply.h>
#include <unity/api/scopes/internal/ice_middleware/IceObjectProxy.h>
#include <unity/api/scopes/internal/ice_middleware/IceReplyProxyFwd.h>
#include <unity/api/scopes/internal/MWReply.h>

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

class IceReply : public virtual IceObjectProxy, public MWReply
{
public:
    IceReply(IceMiddleware* mw_base, middleware::ReplyPrx const& p) noexcept;
    virtual ~IceReply() noexcept;

    virtual void push(std::string const& result) override;
    virtual void finished() override;
};

} // namespace ice_middleware

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

#endif

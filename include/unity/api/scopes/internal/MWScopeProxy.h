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

#ifndef UNITY_API_SCOPES_INTERNAL_MWSCOPEPROXY_H
#define UNITY_API_SCOPES_INTERNAL_MWSCOPEPROXY_H

#include <unity/api/scopes/internal/MWReplyProxy.h>

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

class MWScopeProxy : public MWObjectProxy
{
public:
    UNITY_DEFINES_PTRS(MWScopeProxy);

    virtual ~MWScopeProxy() noexcept;

    virtual void query(std::string const& q, MWReplyProxy::SPtr const& reply) = 0;

protected:
    MWScopeProxy(MiddlewareBase* mw_base);
};

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

#endif

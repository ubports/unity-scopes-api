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

#ifndef UNITY_API_SCOPES_INTERNAL_MWSCOPE_H
#define UNITY_API_SCOPES_INTERNAL_MWSCOPE_H

#include <scopes/internal/MWObjectProxy.h>
#include <scopes/internal/MWReplyProxyFwd.h>
#include <scopes/internal/MWScopeProxyFwd.h>
#include <scopes/QueryCtrlProxyFwd.h>
#include <scopes/Result.h>
#include <scopes/Variant.h>

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

class MWScope : public virtual MWObjectProxy
{
public:
    virtual ~MWScope() noexcept;

    virtual QueryCtrlProxy create_query(std::string const& q, VariantMap const& hints, MWReplyProxy const& reply) = 0;
    virtual QueryCtrlProxy preview(Result const& result, VariantMap const& hints, MWReplyProxy const& reply) = 0;

protected:
    MWScope(MiddlewareBase* mw_base);
};

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

#endif

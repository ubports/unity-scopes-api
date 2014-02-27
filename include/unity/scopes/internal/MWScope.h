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

#ifndef UNITY_SCOPES_INTERNAL_MWSCOPE_H
#define UNITY_SCOPES_INTERNAL_MWSCOPE_H

#include <unity/scopes/internal/MWObjectProxy.h>
#include <unity/scopes/internal/MWReplyProxyFwd.h>
#include <unity/scopes/internal/MWScopeProxyFwd.h>
#include <unity/scopes/QueryCtrlProxyFwd.h>
#include <unity/scopes/CannedQuery.h>
#include <unity/scopes/Variant.h>

namespace unity
{

namespace scopes
{

namespace internal
{

class MWScope : public virtual MWObjectProxy
{
public:
    virtual ~MWScope();

    virtual QueryCtrlProxy search(CannedQuery const& query, VariantMap const& hints, MWReplyProxy const& reply) = 0;
    virtual QueryCtrlProxy activate(VariantMap const& result, VariantMap const& hints, MWReplyProxy const& reply) = 0;
    virtual QueryCtrlProxy perform_action(VariantMap const& result, VariantMap const& hints, std::string const& widget_id, std::string const& action_id, MWReplyProxy const& reply) = 0;
    virtual QueryCtrlProxy preview(VariantMap const& result, VariantMap const& hints, MWReplyProxy const& reply) = 0;

protected:
    MWScope(MiddlewareBase* mw_base);
};

} // namespace internal

} // namespace scopes

} // namespace unity

#endif

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

#ifndef UNITY_SCOPES_INTERNAL_QUERYCTRLIMPL_H
#define UNITY_SCOPES_INTERNAL_QUERYCTRLIMPL_H

#include <unity/scopes/internal/MWQueryCtrlProxyFwd.h>
#include <unity/scopes/internal/MWReplyProxyFwd.h>
#include <unity/scopes/internal/ObjectImpl.h>
#include <unity/scopes/QueryCtrl.h>

namespace unity
{

namespace scopes
{

namespace internal
{

// Proxy used by a scope to push results for a query.
// To indicate that it has sent the last result, the scope must call finished().
// Subsequent calls to finished() are ignored.
// Calls to push() after finished() was called are ignored.
// If the proxy goes out of scope before finished was called, it implicitly calls finished().

class QueryCtrlImpl : public virtual unity::scopes::QueryCtrl, public virtual ObjectImpl
{
public:
    QueryCtrlImpl(MWQueryCtrlProxy const& ctrl_proxy, MWReplyProxy const& reply_proxy);
    virtual ~QueryCtrlImpl();

    void cancel();

    static QueryCtrlProxy create(MWQueryCtrlProxy const& ctrl_proxy, MWReplyProxy const& reply_proxy);

private:
    MWQueryCtrlProxy fwd() const;

    MWReplyProxy reply_proxy_;
};

} // namespace internal

} // namespace scopes

} // namespace unity

#endif

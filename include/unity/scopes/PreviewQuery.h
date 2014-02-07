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

#ifndef UNITY_SCOPES_PREVIEWQUERYBASE_H
#define UNITY_SCOPES_PREVIEWQUERYBASE_H

#include <unity/scopes/QueryCtrlProxyFwd.h>
#include <unity/scopes/ReplyProxyFwd.h>
#include <unity/scopes/ScopeProxyFwd.h>
#include <unity/scopes/Variant.h>
#include <unity/scopes/QueryBase.h>

#include <unity/SymbolExport.h>
#include <unity/util/DefinesPtrs.h>
#include <unity/util/NonCopyable.h>

namespace unity
{

namespace scopes
{

namespace internal
{

class QueryBaseImpl;
class QueryObject;

} // namespace internal

// Abstract server-side base interface for a query that is executed inside a scope.

// TODO: documentation

class UNITY_API PreviewQuery: public QueryBase
{
public:
    NONCOPYABLE(PreviewQuery);
    UNITY_DEFINES_PTRS(PreviewQuery);

    /// Invoked when a PreviewQuery is run, use the provided proxy to push data
    virtual void run(PreviewReplyProxy const& reply) = 0;         // Called by the run time to start this query

    // TODO: Add a method for subpreview request?

    /// @cond
    virtual ~PreviewQuery();
    /// @endcond

protected:
    /// @cond
    PreviewQuery();
    /// @endcond
};

} // namespace scopes

} // namespace unity

#endif

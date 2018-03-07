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

#pragma once

#include <unity/scopes/QueryCtrlProxyFwd.h>
#include <unity/scopes/ReplyProxyFwd.h>
#include <unity/scopes/ScopeProxyFwd.h>
#include <unity/scopes/Variant.h>

#include <unity/util/DefinesPtrs.h>
#include <unity/util/NonCopyable.h>

namespace unity
{

namespace scopes
{

class SearchMetadata;

namespace internal
{

class QueryBaseImpl;
class QueryObject;
class ScopeObject;

} // namespace internal


/**
\brief Abstract server-side base interface for a query that is executed inside a scope.
\see SearchQueryBase, PreviewQueryBase, ActivationQueryBase
*/

class QueryBase
{
public:
    /// @cond
    NONCOPYABLE(QueryBase);
    UNITY_DEFINES_PTRS(QueryBase);
    /// @endcond

    /**
    \brief Called by the scopes runtime when the query originator
    cancels a query.

    Your implementation of this method should ensure that the scope stops
    processing the current query as soon as possible. Any calls to a `push()` method
    once a query is cancelled are ignored, so continuing to push after cancellation
    only wastes CPU cycles. (`push()` returns `false` once a query is cancelled or
    exceeds its cardinality limit.)
    */
    virtual void cancelled() = 0;                          // Originator cancelled the query

    /**
    \brief Check whether this query is still valid.

    valid() returns false if this query is finished or was cancelled earlier. Note that it is possible
    that the runtime may call SearchQueryBase::run(), ActivationQueryBase::activate(), or PreviewQueryBase::run()
    \a after cancelled() was called. Your implementation of these methods should check whether the query is still
    valid and, if not, do nothing.

    This method is provided mainly for convenience: it can be used in your s `run()` or `activate()` implementation to
    avoid doing a lot of work setting up a query that was cancelled earlier. Note that, because cancellation
    can happen at any time during query execution, your implementation should always test the return value
    of `push()`. If `push()` returns `false`, the query was either cancelled or exceeded its cardinality limit.
    Either way, there is no point in continuing to push more results because, once `push()` returns `false`,
    the scopes runtime discards all subsequent results for the query.
    */
    bool valid() const;

    /**
    \brief Returns a dictionary with the scope's current settings.

    Instead of storing the return value, it is preferable to call settings()
    each time your implementation requires a settings value. This ensures
    that, if a user changes settings while the scope is running, the new settings
    take effect with the next query, preview, and so on.

    \note The settings are available only after this QueryBase is instantiated; do not
    call this method from the constructor!

    \return The scope's current settings.
    */
    unity::scopes::VariantMap settings() const;

    /// @cond
    virtual ~QueryBase();
    /// @endcond

protected:
    /// @cond
    QueryBase(internal::QueryBaseImpl* impl);
    void cancel();
    std::unique_ptr<internal::QueryBaseImpl> p;
    /// @endcond

private:
    friend class internal::QueryObject;                    // So QueryObject can call cancel()
    friend class internal::ScopeObject;                    // So ScopeObject can call set_department_id()
};

} // namespace scopes

} // namespace unity

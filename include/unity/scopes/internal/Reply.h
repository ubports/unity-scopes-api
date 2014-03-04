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

#ifndef UNITY_SCOPES_INTERNAL_REPLY_H
#define UNITY_SCOPES_INTERNAL_REPLY_H

#include <unity/scopes/ObjectProxy.h>
#include <unity/scopes/Reply.h>
#include <unity/scopes/ReplyProxyFwd.h>

namespace unity
{

namespace scopes
{

namespace internal
{
class QueryObject;
class ReplyImpl;

/**
\brief Reply allows the results of a query to be sent to the source of the query.
*/

class Reply : public virtual unity::scopes::Reply, protected ObjectProxy
{
public:

    /**
    \brief Informs the source of a query that the query results are complete.

    Calling finished() informs the source of a query that the final result for the query
    was sent, that is, that the query is complete.
    The scope application code is responsible for calling finished() once it has sent the
    final result for a query.
    Multiple calls to finished() and calls to error() after finished() was called are ignored.
    The destructor implicitly calls finished() if a Reply goes out of scope without
    a prior call to finished().
    */
    void finished() const override;

    /**
    \brief Informs the source of a query that the query was terminated due to an error.

    Multiple calls to error() and calls to finished() after error() was called are ignored.
    \param ex An exception_ptr indicating the cause of the error. If ex is a `std::exception`,
              the return value of `what()` is made available to the query source. Otherwise,
              the query source receives `"unknown exception"`.
    */
    void error(std::exception_ptr ex) const override;

    /**
    \brief Destroys a Reply.

    If a Reply goes out of scope without a prior call to finished(), the destructor implicitly calls finished().
    */
    ~Reply();

protected:
    /// @cond
    Reply(internal::ReplyImpl* impl);         // Instantiated only by ReplyImpl
    friend class internal::ReplyImpl;

    internal::ReplyImpl* fwd() const;

    // TODO: This should be a member of the impl, not of the public class
    std::shared_ptr<internal::QueryObject> qo; // Points at the corresponding QueryObject, so we can
                                               // forward cancellation.
    /// @endcond
};

} // namespace internal

} // namespace scopes

} // namespace unity

#endif

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

#ifndef UNITY_API_SCOPES_RECEIVERBASE_H
#define UNITY_API_SCOPES_RECEIVERBASE_H

#include <unity/util/DefinesPtrs.h>
#include <unity/util/NonCopyable.h>
#include <scopes/Category.h>
#include <scopes/ReceiverBase.h>

#include <string>

namespace unity
{

namespace api
{

namespace scopes
{

class ResultItem;

/**
\brief Abstract base class to receive the results of a query.
TODO: fix doc
The scope application code must instantiate a class derived from ReceiverBase and pass that instance as
a parameter to the Scope::query() method. Once a query is sent, the scopes run time repeatedly
invokes the push() method, once for each result returned by the query. Once a query is complete,
the finished() method is called once, to inform the caller that the query is complete.

Calls to push() and finished() are made by an arbitrary thread.

// TODO: add doc for thread pool and concurrent calls to push()
*/

class UNITY_API ReceiverBase : private util::NonCopyable
{
public:
    /// @cond
    UNITY_DEFINES_PTRS(ReceiverBase);

    virtual ~ReceiverBase() noexcept;
    /// @endcond

    /**
    \brief Called once by the scopes run time for each result that is returned by a query().
    */
    virtual void push(ResultItem result) = 0;

    /**
    \brief Called once by the scopes run time for each category that is returned by a query().
    The default implementation does nothing. Receipt of categories may be interleaved with
    the receipt of results, that is, there is no guarantee that the complete set of categories
    will be provided before the first query result.

    If push() throws an exception, the scopes run time calls 
    */
    virtual void push(Category::SCPtr category);

    /**
    \brief Indicates the cause of a call to finished().
    The Error enumerator indicates that a query terminated abnormally, for example,
    because a scope could not be reached over the network or terminated an query
    abnormally.
    */
    enum Reason { Finished, Cancelled, Error };

    /**
    \brief Called once by the scopes run time after the final result for a query() was sent.
    Exceptions thrown from finished() are ignored.
    \param r Indicates the cause for the call to finished().
    */
    virtual void finished(Reason r) = 0;

protected:
    /// @cond
    ReceiverBase();
    /// @endcond
};

/**
\brief Convenience function to convert a ReceiverBase::Reason enumerator to a string.
\return Possible return values are "finished", "cancelled", and "error".
*/
char const* to_string(ReceiverBase::Reason reason);

} // namespace scopes

} // namespace api

} // namespace unity

#endif

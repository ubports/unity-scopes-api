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

#ifndef UNITY_API_SCOPES_RECEIVERBASE_H
#define UNITY_API_SCOPES_RECEIVERBASE_H

#include <unity/util/DefinesPtrs.h>
#include <unity/util/NonCopyable.h>
#include <scopes/Category.h>

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
a parameter to the ScopeProxy::query() method. Once a query is sent, the scopes run time repeatedly
invokes the push() method, once for each result returned by the query. Once a query is complete,
the finished() method is called once, to inform the caller that the query is complete.
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
    Default implementation does nothing.
    */

    virtual void push(Category::SCPtr category);

    /**
    \brief Called once by the scopes run time after the final result for a query() was sent.
    */
    virtual void finished() = 0;

protected:
    /// @cond
    ReceiverBase();
    /// @endcond
};

} // namespace scopes

} // namespace api

} // namespace unity

#endif

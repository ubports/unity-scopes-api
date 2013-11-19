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

#ifndef UNITY_API_SCOPES_REPLYBASE_H
#define UNITY_API_SCOPES_REPLYBASE_H

#include <unity/util/DefinesPtrs.h>
#include <unity/util/NonCopyable.h>

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
The scope application code must instantiate a class derived from ReplyBase and pass that instance as
a parameter to the ScopeProxy::query() method. Once a query is sent, the scopes run time repeatedly
invokes the push() method, once for each result returned by the query. Once a query is complete,
the finished() method is called once, to inform the caller that the query is complete.
*/

class UNITY_API ReplyBase : private util::NonCopyable
{
public:
    /// @cond
    UNITY_DEFINES_PTRS(ReplyBase);

    virtual ~ReplyBase() noexcept;
    /// @endcond

    /**
    \brief Called once by the scopes run time for each result that is returned by a query().
    */
    virtual void push(ResultItem const& result) = 0;

    /**
    \brief Called once by the scopes run time after the final result for a query() was sent.
    */
    virtual void finished() = 0;

protected:
    /// @cond
    ReplyBase();
    /// @endcond
};

} // namespace scopes

} // namespace api

} // namespace unity

#endif

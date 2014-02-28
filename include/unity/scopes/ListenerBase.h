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

#ifndef UNITY_SCOPES_LISTENERBASE_H
#define UNITY_SCOPES_LISTENERBASE_H

#include <unity/util/DefinesPtrs.h>
#include <unity/util/NonCopyable.h>

#include <string>

namespace unity
{

namespace scopes
{

//TODO: fix doc
/**
\brief Abstract base class to receive the results of a query.

The scope application code must instantiate a class derived from ListenerBase and pass that instance as
a parameter to the Scope::search() method. Once a query is sent, the scopes run time repeatedly
invokes the push() method, once for each result returned by the query. Once a query is complete,
the run time calls the finished() method once, to inform the caller that the query is complete.

Calls to push() and finished() are made by an arbitrary thread.
*/
// TODO: add doc for thread pool and concurrent calls to push()

class ListenerBase
{
public:
    /// @cond
    NONCOPYABLE(ListenerBase);
    UNITY_DEFINES_PTRS(ListenerBase);

    virtual ~ListenerBase();
    /// @endcond

    /**
    \brief Indicates the cause of a call to finished().

    The `Error` enumerator indicates that a query terminated abnormally, for example,
    because a scope could not be reached over the network or explicitly reported an error.
    */
    enum Reason { Finished, Cancelled, Error };

    /**
    \brief Called once by the scopes run time after the final result for a query() was sent.

    Exceptions thrown from finished() are ignored.
    \param r Indicates the cause for the call to finished().
    \param error_message If `r` is set to `Error`, `error_message` contains further details.
           Otherwise, `error_message` is the empty string.
    */
    virtual void finished(Reason r, std::string const& error_message) = 0;

protected:
    /// @cond
    ListenerBase();
    /// @endcond
};

/**
\brief Convenience function to convert a ListenerBase::Reason enumerator to a string.
\return Possible return values are "finished", "cancelled", and "error".
*/
char const* to_string(ListenerBase::Reason reason);

} // namespace scopes

} // namespace unity

#endif

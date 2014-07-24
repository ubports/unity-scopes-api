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

#include <unity/scopes/Reply.h>
#include <unity/util/DefinesPtrs.h>
#include <unity/util/NonCopyable.h>

#include <string>

namespace unity
{

namespace scopes
{

/**
\brief Abstract base class to be notified of request completion
(such as a query or activation request).

\see ActivationListenerBase
\see PreviewQueryBase
\see SearchListenerBase
*/

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
    \brief Called once by the scopes run time after the final result for a request was sent.

    Calls to finished() are made by an arbitrary thread.

    Exceptions thrown from finished() are ignored.
    \param r Indicates the cause for the call to finished().
    \param error_message If `r` is set to `Error`, `error_message` contains further details.
           Otherwise, `error_message` is the empty string.
    */
    virtual void finished(Reason r, std::string const& error_message) = 0;

    /**
    \brief Called by the scopes run time each time a scope reports additional reply info.

    More than one info() call can arrive during processing of a single query.

    Calls to info() are made by an arbitrary thread.

    Exceptions thrown from info() are ignored.
    \param w Indicates the cause for the call to info().
    \param warning_message Contains further details about the info code (optional).
    */
    virtual void info(Reply::InfoCode w, std::string const& warning_message);

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

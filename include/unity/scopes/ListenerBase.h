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

#include <unity/scopes/CompletionDetails.h>
#include <unity/scopes/OperationInfo.h>
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
    \brief Called once by the scopes run time after the final result for a request was sent.

    Calls to finished() are made by an arbitrary thread.

    Exceptions thrown from finished() are ignored.
    \param details Contains details about the completion status of a query as well as any additional
    information regarding the operation of the request.
    */
    virtual void finished(CompletionDetails const& details) = 0;

    /**
    \brief Called by the scopes run time each time a scope reports additional information about the
    reply to a query.

    More than one info() call can arrive during processing of a single query.

    Calls to info() are made by an arbitrary thread.

    Exceptions thrown from info() are ignored.
    \param op_info Contains all details of the information being reported.
    */
    virtual void info(OperationInfo const& op_info);

protected:
    /// @cond
    ListenerBase();
    /// @endcond
};

} // namespace scopes

} // namespace unity

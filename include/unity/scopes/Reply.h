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

#ifndef UNITY_SCOPES_REPLY_H
#define UNITY_SCOPES_REPLY_H

#include <unity/scopes/Object.h>
#include <unity/scopes/ReplyProxyFwd.h>

#include <exception>

namespace unity
{

namespace scopes
{

/**
\brief Allows query termination to be sent to the source of a query.
*/

class Reply : public virtual Object
{
public:
    /**
    \brief Informs the source of a query that the query results are complete.

    Calling finished() informs the source of a query that the final result for the query
    was sent, that is, that the query is complete.
    Multiple calls to finished() and calls to error() after finished() was called are ignored.
    The destructor implicitly calls finished() if a Reply goes out of scope without
    a prior call to finished(). Similarly, QueryBase::run() implicitly calls finished() when
    it returns, provided there are no more reply proxies in scope. In other words, calling
    finished() is optional. The scopes run time ensures that the call happens automatically,
    either when the last reply proxy goes out of scope, or when QueryBase::run() returns
    (whichever happens last).
    */
    virtual void finished() = 0;

    /**
    \brief Informs the source of a query that the query was terminated due to an error.

    Multiple calls to error() and calls to finished() after error() was called are ignored.
    \param ex An exception_ptr indicating the cause of the error. If ex is a `std::exception`,
              the return value of `what()` is made available to the query source. Otherwise,
              the query source receives `"unknown exception"`.
    */
    virtual void error(std::exception_ptr ex) = 0;

    /**
    \brief Indicates the cause of a call to info().

    For example, the `NoInternet` enumerator indicates that a scope requires access to the internet
    in order to properly evaluate its results but currently does not have internet connectivity.
    */
    enum InfoCode
    {
        Unknown,                // Scope used a code that isn't known to the client-side run-time
        NoInternet,             // Scope had no internet access
        PoorInternet,           // Slow or incomplete internet results (e.g. timeout)
        NoLocationData,         // No location data was available
        InaccurateLocationData, // Location data was available, but "fuzzy"
        ResultsIncomplete,      // Results are incomplete (e.g. not all data sources could be reached)
        DefaultSettingsUsed,    // Scope used default settings; results may be better with explicit settings
        SettingsProblem         // Scope needed some settings that were not provided (e.g. URL for data source)
    };

    /**
    \brief Informs the source of a query that additional info regarding the query operation is
    available.

    Calling info() does not terminate the query, it simply informs the source that something
    interesting occured during execution of the query.

    Multiple calls to info() for each condition encountered are legal.
    \param info_code Indicates the cause for the call to info().
    \param info_message Contains further details about the info code (optional).
    */
    virtual void info(InfoCode info_code, std::string const& info_message = "") = 0;

    /**
    \brief Destroys a Reply.

    If a Reply goes out of scope without a prior call to finished(), the destructor implicitly calls finished(),
    provided QueryBase::run() has returned.
    */
    virtual ~Reply();

protected:
    /// @cond
    Reply();
    /// @endcond
};

} // namespace scopes

} // namespace unity

#endif

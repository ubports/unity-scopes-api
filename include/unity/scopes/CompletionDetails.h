/*
 * Copyright (C) 2014 Canonical Ltd
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
 * Authored by: Marcus Tomlinson <marcus.tomlinson@canonical.com>
 */

#ifndef UNITY_SCOPES_COMPLETIONDETAILS_H
#define UNITY_SCOPES_COMPLETIONDETAILS_H

#include <unity/scopes/OperationInfo.h>
#include <unity/util/NonCopyable.h>

#include <list>
#include <memory>
#include <string>

namespace unity
{

namespace scopes
{

namespace internal
{

class CompletionDetailsImpl;

}

/**
 \brief blah.
*/

class CompletionDetails final
{
public:
    /// @cond
    NONCOPYABLE(CompletionDetails);
    /// @endcond

    /**
    \brief Indicates the completion status for a query.

    The `Error` enumerator indicates that a query terminated abnormally, for example, a scope could
    not be reached over the network or explicitly reported an error.
    */
    enum CompletionStatus
    {
      OK,
      Cancelled,
      Error
    };

    /**
    \brief Create CompletionDetails with the given completion status code.
    \param status Indicates the completion status of the query.
    */
    CompletionDetails(CompletionStatus status);

    /**
    \brief Create CompletionDetails with the given completion status code and details about the
    query operation.
    \param status Indicates the completion status of the query.
    \param details Contains additional information regarding the query operation.
    */
    CompletionDetails(CompletionStatus status, std::list<OperationInfo> const& details);

    /// @cond
    ~CompletionDetails();
    /// @endcond

    /**
    \brief Get the completion status code.
    \return Enum indicating the completion status of the query.
    */
    CompletionStatus status() const noexcept;

    /**
    \brief Get more details about the query operation.
    \return List containing additional information regarding the query operation.
    */
    std::list<OperationInfo> details() const noexcept;

private:
    std::unique_ptr<internal::CompletionDetailsImpl> p;
};

} // namespace scopes

} // namespace unity

#endif

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

#include <memory>
#include <string>
#include <vector>

namespace unity
{

namespace scopes
{

namespace internal
{

class CompletionDetailsImpl;

}

/**
 \brief A container for details about the completion status of a query as well as any additional
 information regarding the operation of the request.
*/

class CompletionDetails final
{
public:
    /**
    \brief Indicates the completion status for a query.

    The `Error` enumerator indicates that a query terminated abnormally, for example, a scope could
    not be reached over the network or explicitly reported an error.
    */
    enum CompletionStatus { OK, Cancelled, Error };

    /**
    \brief Create CompletionDetails with the given completion status.
    \param status Indicates the completion status of the query.
    */
    CompletionDetails(CompletionStatus status);

    /**
    \brief Create CompletionDetails with the given completion status and message.
    \param status Indicates the completion status of the query.
    \param message Contains further details about the completion status.
    */
    CompletionDetails(CompletionStatus status, std::string const& message);

    /**@name Copy and assignment
    Copy and assignment operators (move and non-move versions) have the usual value semantics.
    */
    //{@
    CompletionDetails(CompletionDetails const& other);
    CompletionDetails(CompletionDetails&&);

    CompletionDetails& operator=(CompletionDetails const& other);
    CompletionDetails& operator=(CompletionDetails&&);
    //@}

    /// @cond
    ~CompletionDetails();
    /// @endcond

    /**
    \brief Get the completion status.
    \return Enum indicating the completion status of the query.
    */
    CompletionStatus status() const noexcept;

    /**
    \brief Get the completion message string.
    \return The completion message string.
    */
    std::string message() const;

    /**
    \brief Add additional operation info to the info list.
    \param info Contains details about something of interest that occurs during the operation of a
    query.
    */
    void add_info(OperationInfo const& info) noexcept;

    /**
    \brief Get a list of all additional opertation info.
    \return List containing additional information regarding the operation of the query.
    */
    std::vector<OperationInfo> info_list() const noexcept;

private:
    std::unique_ptr<internal::CompletionDetailsImpl> p;
};

/**
\brief Convenience function to convert a CompletionDetails::CompletionStatus enumerator to a string.
\return Possible return values are "ok", "cancelled", and "error".
*/
char const* to_string(CompletionDetails::CompletionStatus status);

} // namespace scopes

} // namespace unity

#endif

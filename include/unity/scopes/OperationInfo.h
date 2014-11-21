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

#pragma once

#include <memory>
#include <string>

namespace unity
{

namespace scopes
{

namespace internal
{

class OperationInfoImpl;

}

/**
 \brief A container for details about something of interest that occurs during the operation of a
 request.
*/

class OperationInfo final
{
public:
    /**
    \brief Indicates the type of / cause for the information being reported.

    For example, the `NoInternet` enumerator may indicate that access to the internet was required
    in order to properly evaluate a request, but no internet connectivity was available.
    */
    enum InfoCode
    {
        Unknown,                        // A code unknown to the run-time was used
        NoInternet,                     // No internet access
        PoorInternet,                   // Slow or intermittent internet access
        NoLocationData,                 // No location data available
        InaccurateLocationData,         // Location data available, but "fuzzy"
        ResultsIncomplete,              // Results are incomplete (e.g. not all data sources could be reached)
        DefaultSettingsUsed,            // Default settings used; results may be better with explicit settings
        SettingsProblem,                // Some required settings were not provided (e.g. URL for data source)
        LastInfoCode_ = SettingsProblem // Dummy end marker
    };

    /**
    \brief Create OperationInfo with the given info code.
    \param code Indicates the type of / cause for the information.
    */
    OperationInfo(InfoCode code);

    /**
    \brief Create OperationInfo with the given info code and message.
    \param code Indicates the type of / cause for the information.
    \param message Contains further details about the info code.
    */
    OperationInfo(InfoCode code, std::string message);

    /**@name Copy and assignment
    Copy and assignment operators (move and non-move versions) have the usual value semantics.
    */
    //{@
    OperationInfo(OperationInfo const& other);
    OperationInfo(OperationInfo&&);

    OperationInfo& operator=(OperationInfo const& other);
    OperationInfo& operator=(OperationInfo&&);
    //@}

    /// @cond
    ~OperationInfo();
    /// @endcond

    /**
    \brief Get the info code.
    \return Enum indicating the type of info contained.
    */
    InfoCode code() const noexcept;

    /**
    \brief Get the message string.
    \return The message string.
    */
    std::string message() const;

private:
    std::unique_ptr<internal::OperationInfoImpl> p;
};

} // namespace scopes

} // namespace unity

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

#ifndef UNITY_SCOPES_OPERATIONINFO_H
#define UNITY_SCOPES_OPERATIONINFO_H

#include <string>

namespace unity
{

namespace scopes
{

class OperationInfo final
{
public:
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

    OperationInfo(InfoCode code);
    OperationInfo(InfoCode code, std::string message);

    ~OperationInfo();

    InfoCode code() const noexcept;
    std::string message() const noexcept;

private:
    InfoCode code_;
    std::string message_;
};

} // namespace scopes

} // namespace unity

#endif

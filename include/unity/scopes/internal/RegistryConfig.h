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

#include <unity/scopes/internal/ConfigBase.h>

namespace unity
{

namespace scopes
{

namespace internal
{

class RegistryConfig : public ConfigBase
{
public:
    RegistryConfig(std::string const& identity, std::string const& configfile);
    ~RegistryConfig();

    std::string identity() const;
    std::string mw_kind() const;
    std::string mw_configfile() const;
    std::string scope_installdir() const;       // Directory for Canonical scopes
    std::string oem_installdir() const;         // Directory for OEM scope config files
    std::string click_installdir() const;       // Directory for Click scope config files
    std::string scoperunner_path() const;       // Path to scoperunner binary
    int process_timeout() const;                // Milliseconds to wait before scope is considereed non-responsive.
    int debug_process_timeout() const;          // Milliseconds to wait before scope is considereed non-responsive in debug mode.

private:
    std::string identity_;
    std::string mw_kind_;
    std::string mw_configfile_;
    std::string scope_installdir_;
    std::string oem_installdir_;
    std::string click_installdir_;
    std::string scoperunner_path_;
    int process_timeout_;                       // Milliseconds
    int debug_process_timeout_;                 // Milliseconds
};

} // namespace internal

} // namespace scopes

} // namespace unity

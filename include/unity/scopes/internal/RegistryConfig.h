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

#ifndef UNITY_SCOPES_INTERNAL_REGISTRYCONFIG_H
#define UNITY_SCOPES_INTERNAL_REGISTRYCONFIG_H

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
    static constexpr char const* REGISTRY_CONFIG_GROUP = "Registry";

    RegistryConfig(std::string const& identity, std::string const& configfile);
    ~RegistryConfig() noexcept;

    std::string identity() const;
    std::string mw_kind() const;
    std::string endpointdir() const;
    std::string endpoint() const;
    std::string mw_configfile() const;
    std::string scope_installdir() const;       // Directory for Canonical scopes
    std::string oem_installdir() const;         // Directory for OEM scope config files
    std::string scoperunner_path() const;       // Path to scoperunner binary

private:
    std::string identity_;
    std::string mw_kind_;
    std::string endpointdir_;
    std::string endpoint_;
    std::string mw_configfile_;
    std::string scope_installdir_;
    std::string oem_installdir_;
    std::string scoperunner_path_;
};

} // namespace internal

} // namespace scopes

} // namespace unity

#endif

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

#ifndef UNITY_API_SCOPES_INTERNAL_RUNTIMECONFIG_H
#define UNITY_API_SCOPES_INTERNAL_RUNTIMECONFIG_H

#include <scopes/internal/ConfigBase.h>
#include <scopes/Runtime.h>

namespace unity
{

namespace scopes
{

namespace internal
{

class RuntimeConfig : public ConfigBase
{
public:
    static const char* RUNTIME_CONFIG_GROUP;

    RuntimeConfig(std::string const& configfile);
    ~RuntimeConfig() noexcept;

    std::string registry_identity() const;
    std::string registry_configfile() const;
    std::string default_middleware() const;
    std::string default_middleware_configfile() const;
    std::string factory_configfile() const;

private:
    std::string registry_identity_;
    std::string registry_configfile_;
    std::string default_middleware_;
    std::string default_middleware_configfile_;
    std::string factory_configfile_;
};

} // namespace internal

} // namespace scopes

} // namespace unity

#endif

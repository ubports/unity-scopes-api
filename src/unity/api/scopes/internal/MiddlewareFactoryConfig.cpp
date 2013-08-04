/*
 * Copyright (C) 2013 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Michi Henning <michi.henning@canonical.com>
 */

#include <unity/api/scopes/internal/MiddlewareFactoryConfig.h>

using namespace std;

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

const char* MiddlewareFactoryConfig::MIDDLEWARE_FACTORY_CONFIG_GROUP = "MiddlewareFactory";

MiddlewareFactoryConfig::MiddlewareFactoryConfig(string const& configfile) :
    ConfigBase(configfile)
{
    ice_configfile_ = parser()->get_string(MIDDLEWARE_FACTORY_CONFIG_GROUP, "Ice.Configfile");
    rest_configfile_ = parser()->get_string(MIDDLEWARE_FACTORY_CONFIG_GROUP, "REST.Configfile");
}

MiddlewareFactoryConfig::~MiddlewareFactoryConfig() noexcept
{
}

string MiddlewareFactoryConfig::ice_configfile() const
{
    return ice_configfile_;
}

string MiddlewareFactoryConfig::rest_configfile() const
{
    return rest_configfile_;
}
} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

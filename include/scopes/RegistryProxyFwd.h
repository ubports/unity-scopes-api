/*
 * Copyright (C) 2013 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the Lesser GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * Lesser GNU General Public License for more details.
 *
 * You should have received a copy of the Lesser GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Michi Henning <michi.henning@canonical.com>
 */

#ifndef UNITY_API_SCOPES_REGISTRYPROXYFWD_H
#define UNITY_API_SCOPES_REGISTRYPROXYFWD_H

#include <scopes/RegistryProxyFwd.h>

#include <memory>

namespace unity
{

namespace api
{

namespace scopes
{

class Registry;
typedef std::shared_ptr<Registry> RegistryProxy;

} // namespace scopes

} // namespace api

} // namespace unity

#endif

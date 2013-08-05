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

#include <unity/api/scopes/ScopeExceptions.h>
#include <unity/Exception.h>
#include <unity/ExceptionImplBase.h>

using namespace std;

namespace unity
{

namespace api
{

namespace scopes
{

MiddlewareException::
MiddlewareException(string const& reason)
    : Exception(make_shared<unity::ExceptionImplBase>(this, reason))
{
}

MiddlewareException::
MiddlewareException(MiddlewareException const&) = default;

//! @cond

MiddlewareException&
MiddlewareException::
operator=(MiddlewareException const&) = default;


MiddlewareException::
~MiddlewareException() noexcept = default;

//! @endcond

char const*
MiddlewareException::
what() const noexcept
{
    return "unity::api::scopes::MiddlewareException";
}

exception_ptr
MiddlewareException::
self() const
{
    return make_exception_ptr(*this);
}

ConfigException::
ConfigException(string const& reason)
    : Exception(make_shared<unity::ExceptionImplBase>(this, reason))
{
}

ConfigException::
ConfigException(ConfigException const&) = default;

//! @cond

ConfigException&
ConfigException::
operator=(ConfigException const&) = default;


ConfigException::
~ConfigException() noexcept = default;

//! @endcond

char const*
ConfigException::
what() const noexcept
{
    return "unity::api::scopes::ConfigException";
}

exception_ptr
ConfigException::
self() const
{
    return make_exception_ptr(*this);
}

} // namespace scopes

} // namespace api

} // namespace unity

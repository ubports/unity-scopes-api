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

#include <unity/scopes/ScopeExceptions.h>

using namespace std;

namespace unity
{

namespace scopes
{

MiddlewareException::MiddlewareException(string const& reason) :
    Exception("unity::scopes::MiddlewareException", reason)
{
}

MiddlewareException::MiddlewareException(MiddlewareException const&) = default;

//! @cond

MiddlewareException& MiddlewareException::operator=(MiddlewareException const&) = default;


MiddlewareException::~MiddlewareException() noexcept = default;

//! @endcond

exception_ptr MiddlewareException::self() const
{
    return make_exception_ptr(*this);
}

TimeoutException::TimeoutException(string const& reason) :
    Exception("unity::scopes::TimeoutException", reason),
    MiddlewareException(reason)
{
}

TimeoutException::TimeoutException(TimeoutException const&) = default;

//! @cond

TimeoutException& TimeoutException::operator=(TimeoutException const&) = default;


TimeoutException::~TimeoutException() noexcept = default;

//! @endcond

exception_ptr TimeoutException::self() const
{
    return make_exception_ptr(*this);
}

ConfigException::ConfigException(string const& reason) :
    Exception("unity::scopes::ConfigException", reason)
{
}

ConfigException::ConfigException(ConfigException const&) = default;

//! @cond

ConfigException& ConfigException::operator=(ConfigException const&) = default;


ConfigException::~ConfigException() noexcept = default;

//! @endcond

exception_ptr
ConfigException::
self() const
{
    return make_exception_ptr(*this);
}

NotFoundException::NotFoundException(string const& reason, string const& name) :
    Exception("unity::scopes::NotFoundException",
              reason + (reason.empty() ? "" : " ") + "(name = " + name + ")"),
    name_(name)
{
}

NotFoundException::NotFoundException(NotFoundException const&) = default;

//! @cond

NotFoundException& NotFoundException::operator=(NotFoundException const&) = default;


NotFoundException::~NotFoundException() noexcept = default;

//! @endcond

exception_ptr NotFoundException::self() const
{
    return make_exception_ptr(*this);
}

string NotFoundException::name() const
{
    return name_;
}

} // namespace scopes

} // namespace unity

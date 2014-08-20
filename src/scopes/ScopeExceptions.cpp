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

MiddlewareException::MiddlewareException(std::string const& reason) :
    Exception("unity::scopes::MiddlewareException", reason)
{
}

MiddlewareException::MiddlewareException(MiddlewareException const&) = default;

//! @cond

MiddlewareException& MiddlewareException::operator=(MiddlewareException const&) = default;


MiddlewareException::~MiddlewareException() = default;

//! @endcond

exception_ptr MiddlewareException::self() const
{
    return make_exception_ptr(*this);
}

ObjectNotExistException::ObjectNotExistException(std::string const& reason, std::string const& id) :
    Exception("unity::scopes::ObjectNotExistException",
              reason + (reason.empty() ? "" : " ") + "(id = " + id + ")"),
    MiddlewareException("unity::scopes::ObjectNotExistException" +
                        reason + (reason.empty() ? "" : " ") + "(id = " + id + ")"),
    id_(id)
{
}

ObjectNotExistException::ObjectNotExistException(ObjectNotExistException const&) = default;

//! @cond

ObjectNotExistException& ObjectNotExistException::operator=(ObjectNotExistException const&) = default;


ObjectNotExistException::~ObjectNotExistException() = default;

//! @endcond

exception_ptr ObjectNotExistException::self() const
{
    return make_exception_ptr(*this);
}

string ObjectNotExistException::id() const
{
    return id_;
}

TimeoutException::TimeoutException(std::string const& reason) :
    Exception("unity::scopes::TimeoutException", reason),
    MiddlewareException(reason)
{
}

TimeoutException::TimeoutException(TimeoutException const&) = default;

//! @cond

TimeoutException& TimeoutException::operator=(TimeoutException const&) = default;


TimeoutException::~TimeoutException() = default;

//! @endcond

exception_ptr TimeoutException::self() const
{
    return make_exception_ptr(*this);
}

ConfigException::ConfigException(std::string const& reason) :
    Exception("unity::scopes::ConfigException", reason)
{
}

ConfigException::ConfigException(ConfigException const&) = default;

//! @cond

ConfigException& ConfigException::operator=(ConfigException const&) = default;


ConfigException::~ConfigException() = default;

//! @endcond

exception_ptr
ConfigException::
self() const
{
    return make_exception_ptr(*this);
}

NotFoundException::NotFoundException(std::string const& reason, std::string const& name) :
    Exception("unity::scopes::NotFoundException",
              reason + (reason.empty() ? "" : " ") + "(name = " + name + ")"),
    name_(name)
{
}

NotFoundException::NotFoundException(NotFoundException const&) = default;

//! @cond

NotFoundException& NotFoundException::operator=(NotFoundException const&) = default;


NotFoundException::~NotFoundException() = default;

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

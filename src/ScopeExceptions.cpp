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

#include <scopes/ScopeExceptions.h>

#include <unity/ExceptionImplBase.h>

using namespace std;

namespace unity
{

namespace api
{

namespace scopes
{

MiddlewareException::MiddlewareException(string const& reason) :
    Exception(make_shared<unity::ExceptionImplBase>(this, reason))
{
}

MiddlewareException::MiddlewareException(MiddlewareException const&) = default;

//! @cond

MiddlewareException& MiddlewareException::operator=(MiddlewareException const&) = default;


MiddlewareException::~MiddlewareException() noexcept = default;

//! @endcond

char const* MiddlewareException::what() const noexcept
{
    return "unity::api::scopes::MiddlewareException";
}

exception_ptr MiddlewareException::self() const
{
    return make_exception_ptr(*this);
}

ConfigException::ConfigException(string const& reason) :
    Exception(make_shared<unity::ExceptionImplBase>(this, reason))
{
}

ConfigException::ConfigException(ConfigException const&) = default;

//! @cond

ConfigException& ConfigException::operator=(ConfigException const&) = default;


ConfigException::~ConfigException() noexcept = default;

//! @endcond

char const*
ConfigException::what() const noexcept
{
    return "unity::api::scopes::ConfigException";
}

exception_ptr
ConfigException::
self() const
{
    return make_exception_ptr(*this);
}

namespace internal
{

class NotFoundExceptionImpl : public unity::ExceptionImplBase
{
public:
    NotFoundExceptionImpl(NotFoundException const* owner, string const& reason, string const& name) :
        ExceptionImplBase(owner, reason + " (name = " + name + ")"),
        name_(name)
    {
    }
    string name_;
};

} // namespace internal

NotFoundException::NotFoundException(string const& reason, string const& name) :
    Exception(make_shared<internal::NotFoundExceptionImpl>(this, reason, name))
{
}

NotFoundException::NotFoundException(NotFoundException const&) = default;

//! @cond

NotFoundException& NotFoundException::operator=(NotFoundException const&) = default;


NotFoundException::~NotFoundException() noexcept = default;

//! @endcond

char const* NotFoundException::what() const noexcept
{
    return "unity::api::scopes::NotFoundException";
}

exception_ptr NotFoundException::self() const
{
    return make_exception_ptr(*this);
}

string NotFoundException::name() const
{
    return dynamic_cast<const internal::NotFoundExceptionImpl*>(pimpl())->name_;
}

} // namespace scopes

} // namespace api

} // namespace unity

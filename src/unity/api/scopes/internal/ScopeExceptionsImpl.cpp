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

#include <unity/api/scopes/internal/ScopeExceptionsImpl.h>
#include <unity/ExceptionImplBase.h>

using namespace std;

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

MiddlewareExceptionImpl::
MiddlewareExceptionImpl(string const& reason)
    : Exception(make_shared<ExceptionImplBase>(this, reason))
{
}

char const*
MiddlewareExceptionImpl::
what() const noexcept
{
    return "unity::MiddlewareException";
}

ConfigExceptionImpl::
ConfigExceptionImpl(string const& reason)
    : Exception(make_shared<ExceptionImplBase>(this, reason))
{
}

char const*
ConfigExceptionImpl::
what() const noexcept
{
    return "unity::ConfigException";
}
} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

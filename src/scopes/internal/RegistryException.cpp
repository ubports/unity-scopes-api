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

#include <unity/scopes/internal/RegistryException.h>

using namespace std;

namespace unity
{

namespace scopes
{

namespace internal
{

RegistryException::RegistryException(string const& reason) :
    Exception("unity::scopes::RegistryException", reason)
{
}

RegistryException::RegistryException(RegistryException const&) = default;

//! @cond

RegistryException& RegistryException::operator=(RegistryException const&) = default;


RegistryException::~RegistryException() noexcept = default;

//! @endcond

exception_ptr RegistryException::self() const
{
    return make_exception_ptr(*this);
}

} // namespace internal

} // namespace scopes

} // namespace unity

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

#include <unity/api/scopes/Runtime.h>

#include <unity/api/scopes/internal/RuntimeImpl.h>

using namespace std;

namespace unity
{

namespace api
{

namespace scopes
{

constexpr char const* Runtime::DFLT_CONFIGFILE;

//! @cond

Runtime::Runtime(string const& scope_name, string const& configfile) :
    p(internal::RuntimeImpl::create(scope_name, configfile))
{
}

Runtime::~Runtime() noexcept
{
}

//! @endcond

Runtime::UPtr Runtime::create(string const& scope_name, string const& configfile)
{
    return UPtr(new Runtime(scope_name, configfile));
}

void Runtime::destroy()
{
    p->destroy();
}

RegistryProxy::SPtr Runtime::registry() const
{
    return p->registry();
}

} // namespace scopes

} // namespace api

} // namespace unity

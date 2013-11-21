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

#include <scopes/internal/RegistryImpl.h>

#include <scopes/internal/MWRegistry.h>
#include <scopes/ScopeExceptions.h>
#include <unity/UnityExceptions.h>

using namespace std;

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

RegistryImpl::RegistryImpl(MWRegistryProxy const& mw_proxy, RuntimeImpl*) :
    mw_proxy_(mw_proxy)
{
}

RegistryImpl::~RegistryImpl() noexcept
{
}

ScopeProxy RegistryImpl::find(std::string const& scope_name)
{
    return mw_proxy_->find(scope_name);
}

ScopeMap RegistryImpl::list()
{
    return mw_proxy_->list();
}

RegistryProxy RegistryImpl::create(MWRegistryProxy const& mw_proxy, RuntimeImpl* runtime)
{
    return RegistryProxy(new Registry(new RegistryImpl(mw_proxy, runtime)));
}

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

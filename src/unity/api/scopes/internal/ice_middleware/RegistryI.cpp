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

#include <unity/api/scopes/internal/ice_middleware/RegistryI.h>

#include <unity/api/scopes/internal/ice_middleware/IceScopeProxy.h>
#include <unity/api/scopes/internal/RegistryObject.h>

using namespace std;

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

namespace ice_middleware
{

RegistryI::RegistryI(RegistryObject::SPtr const& ro) :
    ro_(ro)
{
    assert(ro);
}

RegistryI::~RegistryI() noexcept
{
}

// Methods below are marked with a
//    // noexcept
// comment. That's because they really are noexcept, but we can't say this in the signature
// because we are deriving from the Slice-generated base.

middleware::ScopePrx RegistryI::find(std::string const& name, Ice::Current const&) // noexcept
{
    try
    {
        MWScopeProxy::SPtr p = ro_->find(name);
        IceScopeProxy::SPtr ice_proxy = dynamic_pointer_cast<IceScopeProxy>(p);
        return ice_proxy ? ice_proxy->proxy() : nullptr;
    }
    catch (...)
    {
        // TODO: Log error
        return nullptr;
    }
}

middleware::ScopeDict RegistryI::list(Ice::Current const&) // noexcept
{
    middleware::ScopeDict sd;
    try
    {
        RegistryObject::MWScopeMap sm = ro_->list();
        for (auto const& it : sm)
        {
            IceScopeProxy::SPtr ice_proxy = dynamic_pointer_cast<IceScopeProxy>(it.second);
            assert(ice_proxy);
            sd[it.first] = ice_proxy->proxy();
        }
    }
    catch (...)
    {
        // TODO: log error
    }
    return sd;
}

} // namespace ice_middleware

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

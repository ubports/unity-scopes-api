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

#ifndef UNITY_API_SCOPES_OBJECTPROXY_H
#define UNITY_API_SCOPES_OBJECTPROXY_H

#include <scopes/ObjectProxyFwd.h>
#include <unity/SymbolExport.h>

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{
class ObjectProxyImpl;
}

// TODO: currently incomplete, so there is no true base proxy. The technique outlined in http://tinyrul.com/3w6elg
// would be suitable to get covariant return for a pimpl() method that returns a shared_ptr. That way, the pimpl()
// method could be protected here in the base, and we could use ladder inheritance for both the hierarchies
// rooted at ObjectProxy and ObjectProxyImpl.

class UNITY_API ObjectProxy
{
public:
    ObjectProxy();
    virtual ~ObjectProxy() noexcept;

private:
    friend class internal::ObjectProxyImpl;

    std::shared_ptr<internal::ObjectProxyImpl> p;
};

} // namespace scopes

} // namespace api

} // namespace unity

#endif

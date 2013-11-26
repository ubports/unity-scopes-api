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

class UNITY_API ObjectProxy
{
public:
    ObjectProxy();
    virtual ~ObjectProxy() noexcept;

protected:
    internal::ObjectProxyImpl* pimpl() const noexcept; // Non-virtual because we can't use covariance with incomplete types
                                                       // Each derived proxy type implements a non-virtual fwd() method
                                                       // that is called from within each operation to down-cast the pimpl().
    ObjectProxy(internal::ObjectProxyImpl*);
    friend class internal::ObjectProxyImpl; // Instantiated only by ObjectProxyImpl

private:
    std::unique_ptr<internal::ObjectProxyImpl> p;
};

} // namespace scopes

} // namespace api

} // namespace unity

#endif

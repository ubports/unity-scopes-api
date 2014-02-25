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

#ifndef UNITY_SCOPES_OBJECTPROXY_H
#define UNITY_SCOPES_OBJECTPROXY_H

#include <unity/scopes/ObjectProxyFwd.h>

namespace unity
{

namespace scopes
{

namespace internal
{
class ObjectProxyImpl;
class RuntimeImpl;
}

class ObjectProxy
{
public:
    ObjectProxy();
    virtual ~ObjectProxy();

    std::string endpoint() const;
    std::string identity() const;
    int64_t timeout() const;      // Timeout in milliseconds, -1 == no timeout

    std::string to_string() const;

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

} // namespace unity

#endif

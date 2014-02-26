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

/**
\brief The root base class for all proxies.
*/

class ObjectProxy
{
public:
    /// @cond
    ObjectProxy();
    virtual ~ObjectProxy();
    /// @endcond

    /**
    \brief Returns the endpoint this proxy connects to.
    */
    std::string endpoint() const;

    /**
    \brief Returns the identity of the target object of this proxy.
    */
    std::string identity() const;

    /**
    \brief Returns the timeout in milliseconds if this proxy is a twoway proxy.
    For oneway proxies and twoway proxies without a timeout, the return value is -1.
    \return The timeout value in milliseconds (-1 if none or timeout does not apply).
    */
    int64_t timeout() const;      // Timeout in milliseconds, -1 == no timeout

    /**
    \brief converts a proxy into its string representation.

    A proxy string can be converted back into a proxy by calling Runtime::string_to_proxy().
    */
    std::string to_string() const;

protected:
    /// @cond
    internal::ObjectProxyImpl* pimpl() const noexcept; // Non-virtual because we can't use covariance with incomplete types
                                                       // Each derived proxy type implements a non-virtual fwd() method
                                                       // that is called from within each operation to down-cast the pimpl().
    ObjectProxy(internal::ObjectProxyImpl*);
    friend class internal::ObjectProxyImpl; // Instantiated only by ObjectProxyImpl
    /// @endcond

private:
    std::unique_ptr<internal::ObjectProxyImpl> p;
};

} // namespace scopes

} // namespace unity

#endif

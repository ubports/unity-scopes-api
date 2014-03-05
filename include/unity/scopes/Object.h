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

#ifndef UNITY_SCOPES_OBJECT_H
#define UNITY_SCOPES_OBJECT_H

#include <unity/scopes/ObjectProxyFwd.h>
#include <unity/util/NonCopyable.h>

namespace unity
{

namespace scopes
{

/**
\brief The root base class for all proxies.
*/

class Object
{
public:
    /// @cond
    NONCOPYABLE(Object);

    virtual ~Object();
    /// @endcond

    /**
    \brief Returns the endpoint this proxy connects to.
    \return The endpoint of the proxy.
    */
    virtual std::string endpoint() = 0;

    /**
    \brief Returns the identity of the target object of this proxy.
    \return The identity of the target of the proxy.
    */
    virtual std::string identity() = 0;

    /**
    \brief Returns the timeout in milliseconds if this proxy is a twoway proxy.

    For oneway proxies and twoway proxies without a timeout, the return value is -1.
    \return The timeout value in milliseconds (-1 if none or timeout does not apply).
    */
    virtual int64_t timeout() = 0;      // Timeout in milliseconds, -1 == no timeout

    /**
    \brief converts a proxy into its string representation.

    A proxy string can be converted back into a proxy by calling Runtime::string_to_proxy().
    \return The string representation of the proxy.
    */
    virtual std::string to_string() = 0;

protected:
    /// @cond
    Object();
    /// @endcond
};

} // namespace scopes

} // namespace unity

#endif

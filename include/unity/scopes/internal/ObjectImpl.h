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

#ifndef UNITY_SCOPES_INTERNAL_OBJECTIMPL_H
#define UNITY_SCOPES_INTERNAL_OBJECTIMPL_H

#include<unity/scopes/internal/MWObjectProxyFwd.h>
#include<unity/scopes/Object.h>

#include <mutex>

namespace unity
{

namespace scopes
{

namespace internal
{

class ObjectImpl : public virtual Object
{
public:
    ObjectImpl(MWProxy const& mw_proxy);
    virtual ~ObjectImpl();

    virtual std::string identity() override;
    virtual std::string category() override;
    virtual std::string endpoint() override;
    virtual int64_t timeout() override;

    virtual std::string to_string() override;

    // Remote operation. Not part of the public API, hence not override.
    virtual void ping();

protected:
    MWProxy proxy();                   // Non-virtual because we cannot use covariance with incomplete types.
                                       // Each derived proxy implements a non-virtual fwd() method
                                       // that is called from within each operation to down-cast the MWProxy.

    void set_proxy(MWProxy const& p);  // Allows a derived proxy to replace mw_proxy_ for aynchronous twoway calls.

    MWProxy mw_proxy_;
    std::mutex proxy_mutex_;           // Protects mw_proxy_

private:
    void check_proxy();                // Throws from operations if mw_proxy_ is null
};

} // namespace internal

} // namespace scopes

} // namespace unity

#endif

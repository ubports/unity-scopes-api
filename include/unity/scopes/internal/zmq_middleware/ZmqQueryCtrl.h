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

#pragma once

#include <unity/scopes/internal/zmq_middleware/ZmqObjectProxy.h>
#include <unity/scopes/internal/zmq_middleware/ZmqQueryCtrlProxyFwd.h>
#include <unity/scopes/internal/MWQueryCtrl.h>

namespace unity
{

namespace scopes
{

namespace internal
{

namespace zmq_middleware
{

class ZmqQueryCtrl : public virtual ZmqObjectProxy, public virtual MWQueryCtrl
{
public:
    ZmqQueryCtrl(ZmqMiddleware* mw_base,
                 std::string const& endpoint,
                 std::string const& identity,
                 std::string const& category);
    virtual ~ZmqQueryCtrl();

    virtual void cancel() override;
    virtual void destroy() override;
};

} // namespace zmq_middleware

} // namespace internal

} // namespace scopes

} // namespace unity

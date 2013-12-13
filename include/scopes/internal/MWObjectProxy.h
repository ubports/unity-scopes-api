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

#ifndef UNITY_API_REPLYS_INTERNAL_MWOBJECTPROXY_H
#define UNITY_API_REPLYS_INTERNAL_MWOBJECTPROXY_H

#include <scopes/internal/MWObjectProxyFwd.h>

namespace unity
{

namespace scopes
{

namespace internal
{

class MiddlewareBase;

// Base class for the MW<something>Proxy hierarchy. We store the pointer to the middleware here,
// which is needed to, for example, register callback objects (such as a Reply object) with the middleware.

class MWObjectProxy
{
public:
    virtual ~MWObjectProxy() noexcept;

    virtual MiddlewareBase* mw_base() const noexcept = 0;

    virtual std::string identity() const = 0;
    virtual std::string endpoint() const = 0;

protected:
    MWObjectProxy(MiddlewareBase* mw_base);

private:
    MiddlewareBase* mw_base_;
};

} // namespace internal

} // namespace scopes

} // namespace unity

#endif

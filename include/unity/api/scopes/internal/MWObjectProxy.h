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

#ifndef UNITY_API_SCOPES_INTERNAL_MWOBJECTPROXY_H
#define UNITY_API_SCOPES_INTERNAL_MWOBJECTPROXY_H

#include <unity/util/DefinesPtrs.h>

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

class MiddlewareBase;

// Base class for all MW<something>Proxy types. Its job is to make the abstract middleware pointer
// available to all proxies, so it is possible to, for example, instantiate and add a new server-side
// object to the middleware (as is needed for callback objects).

class MWObjectProxy
{
public:
    UNITY_DEFINES_PTRS(MWObjectProxy);

    MiddlewareBase* mw_base() const noexcept;

    virtual ~MWObjectProxy() noexcept;

protected:
    MWObjectProxy(MiddlewareBase* mw_base);

private:
    MiddlewareBase* mw_base_;
};

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

#endif

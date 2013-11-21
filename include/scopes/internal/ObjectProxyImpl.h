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

#ifndef UNITY_INTERNAL_OBJECTPROXYIMPL_H
#define UNITY_INTERNAL_OBJECTPROXYIMPL_H

#include<scopes/internal/MWObjectProxyFwd.h>
#include<scopes/ObjectProxyFwd.h>

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

class ObjectProxyImpl final
{
public:
    ObjectProxyImpl(MWProxy const& mw_proxy);
    ~ObjectProxyImpl() noexcept;

private:

    static Proxy create(MWProxy const& mw_proxy);

private:
    MWProxy mw_proxy_;
};

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

#endif

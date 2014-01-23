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

#ifndef UNITY_SCOPES_INTERNAL_INVOKEINFO_H
#define UNITY_SCOPES_INTERNAL_INVOKEINFO_H

#include <string>

namespace unity
{

namespace scopes
{

namespace internal
{

class MiddlewareBase;

// InvokeInfo is passed to all server-side implementation objects. It is intended
// mainly for the implementation of default servants, which need to incarnate
// different middleware object (that is, conceptual object) on a per-request basis.

struct InvokeInfo
{
    std::string const& id;       // Identity of invocation target
    MiddlewareBase*    mw;       // Middleware for this request
};

} // namespace internal

} // namespace scopes

} // namespace unity

#endif

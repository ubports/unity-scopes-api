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

#ifndef UNITY_SCOPES_INTERNAL_SCOPEOBJECTBASE_H
#define UNITY_SCOPES_INTERNAL_SCOPEOBJECTBASE_H

#include <unity/scopes/internal/AbstractObject.h>
#include <unity/scopes/internal/MWQueryCtrlProxyFwd.h>
#include <unity/scopes/internal/MWReplyProxyFwd.h>
#include <unity/scopes/Variant.h>

namespace unity
{

namespace scopes
{

class ScopeBase;

namespace internal
{

class MiddlewareBase;
class RuntimeImpl;

class ScopeObjectBase : public AbstractObject
{
public:
    UNITY_DEFINES_PTRS(ScopeObjectBase);

    virtual MWQueryCtrlProxy create_query(std::string const& q,
                                          VariantMap const& hints,
                                          MWReplyProxy const& reply,
                                          MiddlewareBase* mw_base) = 0;
};

} // namespace internal

} // namespace scopes

} // namespace unity

#endif

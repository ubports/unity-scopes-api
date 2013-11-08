/*
 * Copyright (C) 2013 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Michi Henning <michi.henning@canonical.com>
 */

#ifndef UNITY_API_SCOPES_INTERNAL_SCOPEOBJECT_H
#define UNITY_API_SCOPES_INTERNAL_SCOPEOBJECT_H

#include <scopes/internal/AbstractObject.h>
#include <scopes/internal/MWQueryCtrlProxyFwd.h>
#include <scopes/internal/MWReplyProxyFwd.h>
#include <scopes/Variant.h>

#include <string>

namespace unity
{

namespace api
{

namespace scopes
{

class ScopeBase;

namespace internal
{

class MiddlewareBase;
class RuntimeImpl;

// A ScopeObject sits in between the incoming requests from the middleware layer and the
// ScopeBase-derived implementation provided by the scope. It forwards incoming
// queries to the actual scope. This allows us to intercept all queries for a scope.

class ScopeObject final : public AbstractObject
{
public:
    UNITY_DEFINES_PTRS(ScopeObject);

    ScopeObject(RuntimeImpl* runtime, ScopeBase* scope_base_);
    virtual ~ScopeObject() noexcept;

    // Remote operation implementations
    MWQueryCtrlProxy create_query(std::string const& q,
                                  VariantMap const& hints,
                                  MWReplyProxy const& reply,
                                  MiddlewareBase* mw_base);

private:
    RuntimeImpl* const runtime_;
    ScopeBase* const scope_base_;
};

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

#endif

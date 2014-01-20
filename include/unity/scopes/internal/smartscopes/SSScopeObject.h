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
 * Authored by: Marcus Tomlinson <marcus.tomlinson@canonical.com>
 */

#ifndef UNITY_SCOPES_INTERNAL_SMARTSCOPES_SSSCOPEOBJECT_H
#define UNITY_SCOPES_INTERNAL_SMARTSCOPES_SSSCOPEOBJECT_H

#include <unity/scopes/internal/AbstractObject.h>
#include <unity/scopes/internal/MWQueryCtrlProxyFwd.h>
#include <unity/scopes/internal/MWReplyProxyFwd.h>
#include <unity/scopes/Variant.h>
#include <unity/scopes/Result.h>

#include <string>

namespace unity
{

namespace scopes
{

class ScopeBase;
class Result;

namespace internal
{

class MiddlewareBase;
class RuntimeImpl;

namespace smartscopes
{

// A SSScopeObject sits in between the incoming requests from the middleware layer and the
// ScopeBase-derived implementation provided by the scope. It forwards incoming
// queries to the actual scope. This allows us to intercept all queries for a scope.

class SSScopeObject final : public AbstractObject
{
public:
    UNITY_DEFINES_PTRS(SSScopeObject);

    SSScopeObject(RuntimeImpl* runtime, ScopeBase* scope_base_);
    virtual ~SSScopeObject() noexcept;

    // Remote operation implementations
    MWQueryCtrlProxy create_query(std::string const& q,
                                  VariantMap const& hints,
                                  MWReplyProxy const& reply,
                                  MiddlewareBase* mw_base);


    MWQueryCtrlProxy activate(Result const& result,
                              VariantMap const& hints,
                              MWReplyProxy const &reply,
                              MiddlewareBase* mw_base);

    MWQueryCtrlProxy preview(Result const& result,
                             VariantMap const& hints,
                             MWReplyProxy const& reply,
                             MiddlewareBase* mw_base);

private:
    RuntimeImpl* const runtime_;
    ScopeBase* const scope_base_;
};

} // namespace smartscopes

} // namespace internal

} // namespace scopes

} // namespace unity

#endif // UNITY_SCOPES_INTERNAL_SMARTSCOPES_SSSCOPEOBJECT_H

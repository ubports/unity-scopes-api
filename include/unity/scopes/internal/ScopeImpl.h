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

#ifndef UNITY_INTERNAL_SCOPEIMPL_H
#define UNITY_INTERNAL_SCOPEIMPL_H

#include <unity/scopes/internal/MWScopeProxyFwd.h>
#include <unity/scopes/internal/ObjectProxyImpl.h>
#include <unity/scopes/QueryCtrlProxyFwd.h>
#include <unity/scopes/ReceiverBase.h>
#include <unity/scopes/ScopeProxyFwd.h>
#include <unity/scopes/Variant.h>

namespace unity
{

namespace scopes
{

namespace internal
{

class RuntimeImpl;

class ScopeImpl : public virtual ObjectProxyImpl
{
public:
    ScopeImpl(MWScopeProxy const& mw_proxy, RuntimeImpl* runtime);
    virtual ~ScopeImpl() noexcept;

    QueryCtrlProxy create_query(std::string const& q, VariantMap const& hints, ReceiverBase::SPtr const& reply) const;

    static ScopeProxy create(MWScopeProxy const& mw_proxy, RuntimeImpl* runtime);

private:
    MWScopeProxy fwd() const;

    RuntimeImpl* const runtime_;
};

} // namespace internal

} // namespace scopes

} // namespace unity

#endif

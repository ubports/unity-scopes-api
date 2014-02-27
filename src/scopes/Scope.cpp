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

#include <unity/scopes/Scope.h>
#include <unity/scopes/ActionMetadata.h>
#include <unity/scopes/internal/ScopeImpl.h>

namespace unity
{

namespace scopes
{

//! @cond

Scope::Scope(internal::ScopeImpl* impl) :
    ObjectProxy(impl)
{
}

Scope::~Scope()
{
}

QueryCtrlProxy Scope::search(std::string const& query_string, SearchMetadata const& metadata, SearchListenerBase::SPtr const& reply) const
{
    return fwd()->search(query_string, metadata, reply);
}

QueryCtrlProxy Scope::search(std::string const& query_string, FilterState const& filter_state, SearchMetadata const& metadata, SearchListenerBase::SPtr const& reply) const
{
    return fwd()->search(query_string, filter_state, metadata, reply);
}

QueryCtrlProxy Scope::search(std::string const& query_string, std::string const& department_id, FilterState const& filter_state, SearchMetadata const& metadata, SearchListenerBase::SPtr const& reply) const
{
    return fwd()->search(query_string, department_id, filter_state, metadata, reply);
}

QueryCtrlProxy Scope::activate(Result const& result, ActionMetadata const& metadata, ActivationListenerBase::SPtr const& reply) const
{
    return fwd()->activate(result, metadata, reply);
}

QueryCtrlProxy Scope::perform_action(Result const& result, ActionMetadata const& metadata, std::string const& widget_id, std::string const& action_id,
        ActivationListenerBase::SPtr const& reply) const
{
    return fwd()->perform_action(result, metadata, widget_id, action_id, reply);
}

QueryCtrlProxy Scope::preview(Result const& result, ActionMetadata const& metadata, PreviewListenerBase::SPtr const& reply) const
{
    return fwd()->preview(result, metadata, reply);
}

internal::ScopeImpl* Scope::fwd() const
{
    return dynamic_cast<internal::ScopeImpl*>(pimpl());
}

//! @endcond

} // namespace scopes

} // namespace unity

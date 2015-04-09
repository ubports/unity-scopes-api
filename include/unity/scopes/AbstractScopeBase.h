/*
 * Copyright (C) 2014 Canonical Ltd
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

#include <unity/scopes/ActivationQueryBase.h>
#include <unity/scopes/ChildScope.h>
#include <unity/scopes/PreviewQueryBase.h>
#include <unity/scopes/Registry.h>
#include <unity/scopes/SearchQueryBase.h>

namespace unity
{

namespace scopes
{

/**
\brief Abstract base class for a scope implementation.

This class allows a scope to be replaced by a mock implementation.
See unity::scopes::ScopeBase for documentation of the member functions.
*/

class AbstractScopeBase
{
public:
/// @cond
    virtual void start(std::string const& scope_id) = 0;
    virtual void stop() = 0;
    virtual void run() = 0;
    virtual SearchQueryBase::UPtr search(CannedQuery const& query, SearchMetadata const& metadata) = 0;
    virtual ActivationQueryBase::UPtr activate(Result const& result, ActionMetadata const& metadata) = 0;
    virtual ActivationQueryBase::UPtr perform_action(Result const& result,
                                                     ActionMetadata const& metadata,
                                                     std::string const& widget_id,
                                                     std::string const& action_id) = 0;
    virtual PreviewQueryBase::UPtr preview(Result const& result, ActionMetadata const& metadata) = 0;
    virtual std::string scope_directory() const = 0;
    virtual std::string cache_directory() const = 0;
    virtual std::string tmp_directory() const = 0;
    virtual unity::scopes::RegistryProxy registry() const = 0;
    virtual VariantMap settings() const = 0;

    virtual ~AbstractScopeBase();

    virtual ChildScopeList find_child_scopes() const = 0;
    virtual ChildScopeList child_scopes() const = 0;

protected:
    AbstractScopeBase();
/// @endcond
};

} // namespace scopes

} // namespace unity

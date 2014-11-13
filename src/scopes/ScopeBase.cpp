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

#include <unity/scopes/ScopeBase.h>
#include <unity/scopes/ActivationQueryBase.h>
#include <unity/scopes/Registry.h>
#include <unity/scopes/internal/ScopeBaseImpl.h>

namespace unity
{

namespace scopes
{

//! @cond

ScopeBase::ScopeBase()
    : p(new internal::ScopeBaseImpl())
{
}

ScopeBase::~ScopeBase()
{
}

//! @endcond

void ScopeBase::start(std::string const&)
{
    // Intentionally empty: default "do nothing" implementation.
}

void ScopeBase::stop()
{
    // Intentionally empty: default "do nothing" implementation.
}

void ScopeBase::run()
{
    // Intentionally empty: default "do nothing" implementation.
}

ActivationQueryBase::UPtr ScopeBase::activate(Result const& result, ActionMetadata const& metadata)
{
    return ActivationQueryBase::UPtr(new ActivationQueryBase(result, metadata)); // default impl returns NotHandled
}

ActivationQueryBase::UPtr ScopeBase::perform_action(Result const& result, ActionMetadata const& metadata, std::string const& widget_id, std::string const& action_id)
{
    return ActivationQueryBase::UPtr(new ActivationQueryBase(result, metadata, widget_id, action_id)); // default impl returns NotHandled
}

void ScopeBase::runtime_version(int& v_major, int& v_minor, int& v_micro) noexcept
{
    v_major = unity::scopes::major_version();
    v_minor = unity::scopes::minor_version();
    v_micro = unity::scopes::micro_version();
}

std::string ScopeBase::scope_directory() const
{
    return p->scope_directory();
}

std::string ScopeBase::cache_directory() const
{
    return p->cache_directory();
}

std::string ScopeBase::app_directory() const
{
    return p->cache_directory();
}

std::string ScopeBase::tmp_directory() const
{
    return p->tmp_directory();
}

RegistryProxy ScopeBase::registry() const
{
    return p->registry();
}

unity::scopes::VariantMap ScopeBase::settings() const
{
    return p->settings();
}

} // namespace scopes

} // namespace unity

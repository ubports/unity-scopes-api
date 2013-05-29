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

#include <unity/api/scopes/ScopeBase.h>
#include <unity/api/scopes/internal/ScopeBaseImpl.h>

namespace unity
{

namespace api
{

namespace scopes
{

/// @cond

ScopeBase::
ScopeBase()
    : p_(new internal::ScopeBaseImpl)
{
}

ScopeBase::
~ScopeBase()
{
}

/// @endcond

void
ScopeBase::
stop()
{
    p_->stop();
}

void
ScopeBase::
run() noexcept
{
    p_->run();
}

void
ScopeBase::
runtime_version(int& v_major, int& v_minor, int& v_micro) noexcept
{
    p_->runtime_version(v_major, v_minor, v_micro);
}

} // namespace scopes

} // namespace api

} // namespace unity

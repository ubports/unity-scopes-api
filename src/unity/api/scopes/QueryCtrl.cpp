/*
 * Copyright (C) 2013 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the Lesser GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * Lesser GNU General Public License for more details.
 *
 * You should have received a copy of the Lesser GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Michi Henning <michi.henning@canonical.com>
 */

#include <unity/api/scopes/QueryCtrl.h>

#include <unity/api/scopes/internal/QueryCtrlImpl.h>

namespace unity
{

namespace api
{

namespace scopes
{

//! @cond

QueryCtrl::QueryCtrl(internal::QueryCtrlImpl* impl)
    : p(impl)
{
}

QueryCtrl::~QueryCtrl() noexcept
{
}

void QueryCtrl::cancel() const
{
    return p->cancel();
}

//! @endcond

} // namespace scopes

} // namespace api

} // namespace unity

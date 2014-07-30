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

#include <unity/scopes/ListenerBase.h>

#include <cassert>

using namespace std;

namespace unity
{

namespace scopes
{

//! @cond

ListenerBase::ListenerBase()
{
}

ListenerBase::~ListenerBase()
{
}

void ListenerBase::info(OperationInfo const&)
{
    // Intentionally stubbed to allow listeners to ignore info messages
}

//! @endcond

} // namespace scopes

} // namespace unity

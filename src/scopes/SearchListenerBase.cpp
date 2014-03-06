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

#include <unity/scopes/SearchListenerBase.h>

namespace unity
{

namespace scopes
{

//! @cond

SearchListenerBase::SearchListenerBase()
{
}

SearchListenerBase::~SearchListenerBase()
{
}

void SearchListenerBase::push(DepartmentList const& /* departments */, std::string const& /* current_department_id */)
{
    // Intentionally empty: "do nothing" default implementation.
}

void SearchListenerBase::push(Filters const& /* filters */, FilterState const& /* filter_state */)
{
    // Intentionally empty: "do nothing" default implementation.
}

void SearchListenerBase::push(Category::SCPtr /* category */)
{
    // Intentionally empty: "do nothing" default implementation.
}

void SearchListenerBase::push(Annotation /* annotation */)
{
    // Intentionally empty: "do nothing" default implementation.
}

//! @endcond

} // namespace scopes

} // namespace unity

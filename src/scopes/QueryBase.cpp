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

#include <unity/scopes/QueryBase.h>

#include <unity/scopes/internal/QueryBaseImpl.h>

using namespace std;

namespace unity
{

namespace scopes
{

QueryBase::QueryBase() :
    p(new internal::QueryBaseImpl)
{
}

QueryBase::~QueryBase() noexcept
{
}

void QueryBase::cancel()
{
    p->cancel();    // Forward cancel to subqueries
    cancelled();    // Inform this query that it was cancelled
}

} // namespace scopes

} // namespace unity

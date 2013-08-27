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

#include <unity/api/scopes/internal/QueryCtrlObject.h>

#include <unity/api/scopes/internal/QueryObject.h>

#include <cassert>

using namespace std;
using namespace unity::api::scopes::internal;

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

QueryCtrlObject::QueryCtrlObject() :
    destroyed_(false)
{
}

QueryCtrlObject::~QueryCtrlObject() noexcept
{
}

void QueryCtrlObject::cancel()
{
    if (destroyed_.exchange(true))
    {
        return;
    }

    QueryObject::SPtr qo = qo_.lock();
    if (qo)
    {
        qo->cancel();
    }
    disconnect();
}

void QueryCtrlObject::destroy()
{
    if (destroyed_.exchange(true))
    {
        return;
    }
    disconnect();
}

// Called by create_query() to tell us what the query facade object
// is. We use the query facade object to forward cancellation.

void QueryCtrlObject::set_query(QueryObject::SPtr const& qo)
{
    assert(!qo_.lock());
    qo_ = qo;
}

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

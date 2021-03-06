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

#include <unity/scopes/internal/QueryCtrlObject.h>

#include <unity/scopes/internal/QueryObject.h>

#include <cassert>

using namespace std;
using namespace unity::scopes::internal;

namespace unity
{

namespace scopes
{

namespace internal
{

QueryCtrlObject::QueryCtrlObject() :
    destroyed_(false)
{
}

QueryCtrlObject::~QueryCtrlObject()
{
}

void QueryCtrlObject::cancel(InvokeInfo const& info)
{
    if (destroyed_.exchange(true))
    {
        return;
    }

    QueryObjectBase::SPtr qo = qo_.lock();
    if (qo)
    {
        qo->cancel(info);
    }
    disconnect();
}

void QueryCtrlObject::destroy(InvokeInfo const& /* info */)
{
    if (destroyed_.exchange(true))
    {
        return;
    }
    disconnect();
}

// Called by search() to tell us what the query facade object
// is. We use the query facade object to forward cancellation.

void QueryCtrlObject::set_query(QueryObjectBase::SPtr const& qo)
{
    assert(!qo_.lock());
    qo_ = qo;
}

} // namespace internal

} // namespace scopes

} // namespace unity

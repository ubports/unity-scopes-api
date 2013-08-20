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

QueryCtrlObject::QueryCtrlObject()
{
}

QueryCtrlObject::~QueryCtrlObject() noexcept
{
}

void QueryCtrlObject::cancel()
{
    // Forward the cancellation to the facade. Converting our weak reference
    // to a strong reference and letting the strong reference go out of scope
    // means that we release the facade. (The facade object may hang around
    // if there are still other references to it.)
    // We pass the shared_ptr to the facade we are invoking on so the facade
    // can construct a reply proxy to pass to cancel.

    QueryObject::SPtr qo = qo_.lock();
    if (qo)
    {
        qo->cancel(qo);
    }
}

void QueryCtrlObject::destroy()
{
    // Nothing to do; calling middleware unregisters us.
}

// Called by create_query() to tell us what the query proxy and facade object
// are. We use the facade object to forward cancellation.

void QueryCtrlObject::set_query(QueryObject::SPtr const& qo)
{
    assert(!qo_.lock());
    qo_ = qo;
}

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

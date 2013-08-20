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

#include <unity/api/scopes/internal/ice_middleware/QueryCtrlI.h>

#include <unity/api/scopes/internal/QueryCtrlObject.h>

#include <Ice/LocalException.h>
#include <Ice/ObjectAdapter.h>

using namespace std;

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

namespace ice_middleware
{

QueryCtrlI::QueryCtrlI(QueryCtrlObject::SPtr const& co) :
    co_(co),
    destroyed_(false)
{
}

QueryCtrlI::~QueryCtrlI()
{
}

// When a query is cancelled, we inform the control
// object of the cancellation and, once that call
// returns, commit suicide.

void QueryCtrlI::cancel(Ice::Current const& c)
{
    cerr << "QueryCtrlI: cancel: " << c.id.name << endl;
    if (destroyed_.exchange(true))
    {
        cerr << "QueryCtrlI: already cancelled/destroyed: " << c.id.name << endl;
        return; // No exception thrown here. It's a oneway invocation, so the exception won't go anywhere.
    }

    try
    {
        co_->cancel();  // Forward cancel request to facade object
    }
    catch (...)
    {
        // TODO: log error
    }

    try
    {
        c.adapter->remove(c.id);
    }
    catch (...)
    {
        // The remove may fail if the query has finished already, so no exception here.
    }
}

// If a query is never cancelled, once it has has pushed its
// last result, the QueryObject instructs us to commit suicide.
// directly.

void QueryCtrlI::destroy(Ice::Current const& c) noexcept
{
    cerr << "QueryCtrlI: destroy: " << c.id.name << endl;
    if (destroyed_.exchange(true))
    {
        cerr << "QueryCtrlI: already cancelled/destroyed: " << c.id.name << endl;
        return; // No exception thrown here. It's a oneway invocation, so the exception won't go anywhere.
    }

    try
    {
        co_->destroy();  // Forward destroy request to facade object
    }
    catch (...)
    {
        // TODO: log error
    }

    try
    {
        c.adapter->remove(c.id);
    }
    catch (...)
    {
        // The remove may fail if the query was cancelled previously, so no exception here.
    }
}

} // namespace ice_middleware

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

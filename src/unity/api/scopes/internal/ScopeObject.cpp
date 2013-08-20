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

#include <unity/api/scopes/internal/ScopeObject.h>

#include <unity/api/scopes/internal/MiddlewareBase.h>
#include <unity/api/scopes/internal/MWQuery.h>
#include <unity/api/scopes/internal/MWQueryCtrlProxyFwd.h>
#include <unity/api/scopes/internal/MWReply.h>
#include <unity/api/scopes/internal/QueryObject.h>
#include <unity/api/scopes/internal/ReplyImpl.h>
#include <unity/api/scopes/ScopeBase.h>
#include <unity/UnityExceptions.h>

#include <cassert>
#include <sstream>

using namespace std;
using namespace unity::api::scopes;
using namespace unity::api::scopes::internal;

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

ScopeObject::ScopeObject(string const& scope_name, ScopeBase* scope_base) :
    scope_name_(scope_name),
    scope_base_(scope_base)
{
    assert(scope_base);
}

ScopeObject::~ScopeObject() noexcept
{
}

MWQueryCtrlProxy ScopeObject::create_query(std::string const& q,
                                           MWReplyProxy const& reply,
                                           MiddlewareBase* mw_base)
{
    if (!reply)
    {
        // We can't assert here because the null proxy may have been sent by a broken client, that is,
        // it can be null because it was sent by the remote end as null. This should never happen but,
        // to be safe, we don't assert, in case someone is running a broken client.

        // TODO: log error about incoming request containing an invalid reply proxy.

        throw LogicException("Scope \"" + scope_name_ + "\": create_query(\"" + q + "\") called with null reply proxy");
    }

    // Ask scope to instantiate a new query.
    QueryBase::SPtr query_base = scope_base_->create_query(q);
    if (!query_base)
    {
        // TODO: log error, scope returned null pointer.
        throw ResourceException("Scope \"" + scope_name_ + "\" returned nullptr from create_query(\"" + q + "\")");
    }

    MWQueryCtrlProxy ctrl_proxy;
    try
    {
        // Instantiate the query ctrl and connect it to the middleware.
        QueryCtrlObject::SPtr co(new QueryCtrlObject);
        ctrl_proxy = mw_base->add_query_ctrl_object(co);

        // Instantiate the query. We tell it what the ctrl is so,
        // when the query completes, it can tell the ctrl object
        // to destroy itself.
        QueryObject::SPtr qo(new QueryObject(query_base, reply, ctrl_proxy));
        MWQueryProxy query = mw_base->add_query_object(qo);

        // We tell the ctrl what the query facade is so, when cancel() is sent
        // to the ctrl, it can forward it to the facade.
        co->set_query(qo);

        // Start the query. We call via the middleware, which calls
        // the run() implementation in a different thread, so we cannot block here.
        // We pass a shared_ptr to the qo to the qo itself, so the qo can hold the reference
        // count high until the run() request arrives in the query via the middleware.
        qo->set_self(qo);
        query->run(reply);
    }
    catch (unity::Exception const& e)
    {
        try
        {
            reply->finished();
        }
        catch (...)
        {
        }
        // TODO: log error
        throw;
    }
    catch (...)
    {
        try
        {
            reply->finished();
        }
        catch (...)
        {
        }
        // TODO: log error
        throw;
    }
    return ctrl_proxy;
}

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

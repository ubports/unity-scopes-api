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

#include <scopes/internal/ScopeObject.h>

#include <scopes/internal/MiddlewareBase.h>
#include <scopes/internal/MWQuery.h>
#include <scopes/internal/MWQueryCtrlProxyFwd.h>
#include <scopes/internal/MWReply.h>
#include <scopes/internal/QueryObject.h>
#include <scopes/internal/RuntimeImpl.h>
#include <scopes/ScopeBase.h>
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

ScopeObject::ScopeObject(RuntimeImpl* runtime, ScopeBase* scope_base) :
    runtime_(runtime),
    scope_base_(scope_base)
{
    assert(runtime);
    assert(scope_base);
}

ScopeObject::~ScopeObject() noexcept
{
}

MWQueryCtrlProxy ScopeObject::create_query(std::string const& q,
                                           VariantMap const& hints,
                                           MWReplyProxy const& reply,
                                           MiddlewareBase* mw_base)
{
    if (!reply)
    {
        // We can't assert here because the null proxy may have been sent by a broken client, that is,
        // it can be null because it was sent by the remote end as null. This should never happen but,
        // to be safe, we don't assert, in case someone is running a broken client.

        // TODO: log error about incoming request containing an invalid reply proxy.

        throw LogicException("Scope \"" + runtime_->scope_name() + "\": create_query(\"" +
                             q + "\") called with null reply proxy");
    }

    // Ask scope to instantiate a new query.
    QueryBase::SPtr query_base = scope_base_->create_query(q, hints);
    if (!query_base)
    {
        // TODO: log error, scope returned null pointer.
        throw ResourceException("Scope \"" + runtime_->scope_name() +
                                "\" returned nullptr from create_query(\"" + q + "\")");
    }

    MWQueryCtrlProxy ctrl_proxy;
    try
    {
        // Instantiate the query ctrl and connect it to the middleware.
        QueryCtrlObject::SPtr co(make_shared<QueryCtrlObject>());
        ctrl_proxy = mw_base->add_query_ctrl_object(co);

        // Instantiate the query. We tell it what the ctrl is so,
        // when the query completes, it can tell the ctrl object
        // to destroy itself.
        QueryObject::SPtr qo(make_shared<QueryObject>(query_base, reply, ctrl_proxy));
        MWQueryProxy query_proxy = mw_base->add_query_object(qo);

        // We tell the ctrl what the query facade is so, when cancel() is sent
        // to the ctrl, it can forward it to the facade.
        co->set_query(qo);

        // Start the query. We call via the middleware, which calls
        // the run() implementation in a different thread, so we cannot block here.
        // We pass a shared_ptr to the qo to the qo itself, so the qo can hold the reference
        // count high until the run() request arrives in the query via the middleware.
        qo->set_self(qo);

        query_proxy->run(reply);
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

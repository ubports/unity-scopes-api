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

#include <unity/scopes/internal/ScopeObject.h>

#include <unity/scopes/internal/ActivationQueryObject.h>
#include <unity/scopes/internal/MWQuery.h>
#include <unity/scopes/internal/MWReply.h>
#include <unity/scopes/internal/PreviewQueryObject.h>
#include <unity/scopes/internal/QueryCtrlObject.h>
#include <unity/scopes/internal/QueryObject.h>
#include <unity/scopes/internal/RuntimeImpl.h>
#include <unity/scopes/ScopeBase.h>
#include <unity/UnityExceptions.h>

#include <cassert>
#include <iostream> // TODO: remove this once logging is added
#include <sstream>

using namespace std;
using namespace unity::scopes;
using namespace unity::scopes::internal;

namespace unity
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

ScopeObject::~ScopeObject()
{
}

MWQueryCtrlProxy ScopeObject::query(MWReplyProxy const& reply, MiddlewareBase* mw_base,
        std::function<QueryBase::SPtr()> const& query_factory_fun,
        std::function<QueryObjectBase::SPtr(QueryBase::SPtr, MWQueryCtrlProxy)> const& query_object_factory_fun)
{
    if (!reply)
    {
        // We can't assert here because the null proxy may have been sent by a broken client, that is,
        // it can be null because it was sent by the remote end as null. This should never happen but,
        // to be safe, we don't assert, in case someone is running a broken client.

        // TODO: log error about incoming request containing an invalid reply proxy.

        throw LogicException("Scope \"" + runtime_->scope_name() + "\": query() called with null reply proxy");
    }

    // Ask scope to instantiate a new query.
    QueryBase::SPtr query_base;
    try
    {
        query_base = query_factory_fun();
        if (!query_base)
        {
            // TODO: log error, scope returned null pointer.
            throw ResourceException("Scope \"" + runtime_->scope_name() + "\" returned nullptr from query_factory_fun()");
        }
    }
    catch (...)
    {
        throw ResourceException("Scope \"" + runtime_->scope_name() + "\" threw an exception from query_factory_fun()");
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
        QueryObjectBase::SPtr qo(query_object_factory_fun(query_base, ctrl_proxy));
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
    catch (std::exception const& e)
    {
        try
        {
            reply->finished(ListenerBase::Error, e.what());
        }
        catch (...)
        {
        }
        cerr << "query(): " << e.what() << endl;
        // TODO: log error
        throw;
    }
    catch (...)
    {
        try
        {
            reply->finished(ListenerBase::Error, "unknown exception");
        }
        catch (...)
        {
        }
        cerr << "query(): unknown exception" << endl;
        // TODO: log error
        throw;
    }
    return ctrl_proxy;
}

MWQueryCtrlProxy ScopeObject::search(CannedQuery const& q,
                                           SearchMetadata const& hints,
                                           MWReplyProxy const& reply,
                                           InvokeInfo const& info)
{
    return query(reply, info.mw,
            [&q, &hints, this]() -> SearchQuery::UPtr {
                 auto search_query = this->scope_base_->search(q, hints);
                 search_query->set_metadata(hints);
                 return search_query;
            },
            [&reply, &hints](QueryBase::SPtr query_base, MWQueryCtrlProxy ctrl_proxy) -> QueryObjectBase::SPtr {
                return make_shared<QueryObject>(query_base, hints.cardinality(), reply, ctrl_proxy);
            }
    );
}

MWQueryCtrlProxy ScopeObject::activate(Result const& result,
                                           ActionMetadata const& hints,
                                           MWReplyProxy const& reply,
                                           InvokeInfo const& info)
{
    return query(reply, info.mw,
            [&result, &hints, this]() -> QueryBase::SPtr {
                return this->scope_base_->activate(result, hints);
            },
            [&reply](QueryBase::SPtr query_base, MWQueryCtrlProxy ctrl_proxy) -> QueryObjectBase::SPtr {
                auto activation_base = dynamic_pointer_cast<ActivationBase>(query_base);
                assert(activation_base);
                return make_shared<ActivationQueryObject>(activation_base, reply, ctrl_proxy);
            }
    );
}

MWQueryCtrlProxy ScopeObject::perform_action(Result const& result,
                                             ActionMetadata const& hints,
                                             std::string const& widget_id,
                                             std::string const& action_id,
                                             MWReplyProxy const &reply,
                                             InvokeInfo const& info)
{
    return query(reply, info.mw,
            [&result, &hints, &widget_id, &action_id, this]() -> QueryBase::SPtr {
                return this->scope_base_->perform_action(result, hints, widget_id, action_id);
            },
            [&reply](QueryBase::SPtr query_base, MWQueryCtrlProxy ctrl_proxy) -> QueryObjectBase::SPtr {
                auto activation_base = dynamic_pointer_cast<ActivationBase>(query_base);
                assert(activation_base);
                return make_shared<ActivationQueryObject>(activation_base, reply, ctrl_proxy);
            }
    );
}

MWQueryCtrlProxy ScopeObject::preview(Result const& result,
                                      ActionMetadata const& hints,
                                      MWReplyProxy const& reply,
                                      InvokeInfo const& info)
{
    return query(reply, info.mw,
            [&result, &hints, this]() -> QueryBase::SPtr {
                return this->scope_base_->preview(result, hints);
            },
            [&reply](QueryBase::SPtr query_base, MWQueryCtrlProxy ctrl_proxy) -> QueryObjectBase::SPtr {
                auto preview_query = dynamic_pointer_cast<PreviewQuery>(query_base);
                assert(preview_query);
                return make_shared<PreviewQueryObject>(preview_query, reply, ctrl_proxy);
            }
    );
}

} // namespace internal

} // namespace scopes

} // namespace unity

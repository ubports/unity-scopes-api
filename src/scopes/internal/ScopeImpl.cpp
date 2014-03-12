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

#include <unity/scopes/internal/ScopeImpl.h>

#include <unity/scopes/ActionMetadata.h>
#include <unity/scopes/internal/ActivationReplyObject.h>
#include <unity/scopes/internal/MiddlewareBase.h>
#include <unity/scopes/internal/MWQueryCtrl.h>
#include <unity/scopes/internal/MWScope.h>
#include <unity/scopes/internal/PreviewReplyObject.h>
#include <unity/scopes/internal/QueryCtrlImpl.h>
#include <unity/scopes/internal/ResultImpl.h>
#include <unity/scopes/internal/ResultReplyObject.h>
#include <unity/scopes/SearchMetadata.h>
#include <unity/UnityExceptions.h>

#include <cassert>
#include <iostream> // TODO: remove this once logging is added

using namespace std;

namespace unity
{

namespace scopes
{

namespace internal
{

ScopeImpl::ScopeImpl(MWScopeProxy const& mw_proxy, RuntimeImpl* runtime, std::string const& scope_id) :
    ObjectImpl(mw_proxy),
    runtime_(runtime),
    scope_id_(scope_id)
{
    assert(runtime);
}

ScopeImpl::~ScopeImpl()
{
}

QueryCtrlProxy ScopeImpl::search(std::string const& query_string,
                                 std::string const& department_id,
                                 FilterState const& filter_state,
                                 SearchMetadata const& metadata,
                                 SearchListenerBase::SPtr const& reply)
{
    CannedQuery query(scope_id_, query_string, department_id);
    query.set_filter_state(filter_state);
    return search(query, metadata, reply);
}

QueryCtrlProxy ScopeImpl::search(std::string const& query_string,
                                 FilterState const& filter_state,
                                 SearchMetadata const& metadata,
                                 SearchListenerBase::SPtr const& reply)
{
    CannedQuery query(scope_id_);
    query.set_query_string(query_string);
    query.set_filter_state(filter_state);
    return search(query, metadata, reply);
}

QueryCtrlProxy ScopeImpl::search(string const& query_string,
                                 SearchMetadata const& metadata,
                                 SearchListenerBase::SPtr const& reply)
{
    CannedQuery query(scope_id_);
    query.set_query_string(query_string);
    return search(query, metadata, reply);
}

QueryCtrlProxy ScopeImpl::search(CannedQuery const& query,
                                 SearchMetadata const& metadata,
                                 SearchListenerBase::SPtr const& reply)
{
cerr << "In search" << endl;
    if (reply == nullptr)
    {
        throw unity::InvalidArgumentException("Scope::search(): invalid SearchListenerBase (nullptr)");
    }

    ReplyObject::SPtr ro(make_shared<ResultReplyObject>(reply, runtime_, to_string(), metadata.cardinality()));
    MWReplyProxy rp = fwd()->mw_base()->add_reply_object(ro);

    // "Fake" QueryCtrlProxy that doesn't have a real MWQueryCtrlProxy yet.
    shared_ptr<QueryCtrlImpl> ctrl = make_shared<QueryCtrlImpl>(nullptr, rp);

    auto send_create_query = [this, query, metadata, rp, ro, ctrl]() -> void
    {
        try
        {
            // Forward the the search() method across the bus. This is a
            // synchronous twoway interaction with the scope, so it can return
            // the QueryCtrlProxy. This may block for some time, for example, if
            // the scope is not running and needs to be activated by the registry first.
cerr << "forwarding query" << endl;
            auto real_ctrl = dynamic_pointer_cast<QueryCtrlImpl>(fwd()->search(query, metadata.serialize(), rp));
cerr << "forwarding query returned" << endl;
            assert(real_ctrl);

            // Call has completed now, so we update the MWQueryCtrlProxy for the fake proxy
            // with the real proxy that was returned.
            auto new_proxy = dynamic_pointer_cast<MWQueryCtrl>(real_ctrl->proxy());
            assert(new_proxy);
cerr << "setting proxy" << endl;
            ctrl->set_proxy(new_proxy);
cerr << "OK, set proxy" << endl;
        }
        catch (std::exception const& e)
        {
            try
            {
cerr << "BAD: " << e.what() << endl;
                ro->finished(ListenerBase::Error, e.what());
            }
            catch (...)
            {
            }
        }
    };

cerr << "search submitting" << endl;
    auto future = runtime_->pool()->submit(send_create_query);
cerr << "pushing future: " << runtime_->future_queue()->size() << endl;
    runtime_->future_queue()->push(move(future));
cerr << "pushed future: " << runtime_->future_queue()->size() << endl;
    return ctrl;
}

QueryCtrlProxy ScopeImpl::activate(Result const& result,
                                   ActionMetadata const& metadata,
                                   ActivationListenerBase::SPtr const& reply)
{
    if (reply == nullptr)
    {
        throw unity::InvalidArgumentException("Scope::activate(): invalid ActivationListenerBase (nullptr)");
    }

    QueryCtrlProxy ctrl;
    ActivationReplyObject::SPtr ro(make_shared<ActivationReplyObject>(reply, runtime_, to_string()));
    try
    {
        MWReplyProxy rp = fwd()->mw_base()->add_reply_object(ro);

        // Forward the activate() method across the bus.
        ctrl = fwd()->activate(result.p->activation_target(), metadata.serialize(), rp);
        assert(ctrl);
    }
    catch (std::exception const& e)
    {
        // TODO: log error
        cerr << "activate(): " << e.what() << endl;
        try
        {
            ro->finished(ListenerBase::Error, e.what());
            throw;
        }
        catch (...)
        {
            cerr << "activate(): unknown exception" << endl;
        }
        throw;
    }
    return ctrl;
}

QueryCtrlProxy ScopeImpl::perform_action(Result const& result,
                                         ActionMetadata const& metadata,
                                         std::string const& widget_id,
                                         std::string const& action_id,
                                         ActivationListenerBase::SPtr const& reply)
{
    if (reply == nullptr)
    {
        throw unity::InvalidArgumentException("Scope::perform_action(): invalid ActivationListenerBase (nullptr)");
    }

    QueryCtrlProxy ctrl;
    try
    {
        // Create a middleware server-side object that can receive incoming
        // push() and finished() messages over the network.
        ActivationReplyObject::SPtr ro(make_shared<ActivationReplyObject>(reply, runtime_, to_string()));
        MWReplyProxy rp = fwd()->mw_base()->add_reply_object(ro);

        // Forward the activate() method across the bus.
        ctrl = fwd()->perform_action(result.p->activation_target(), metadata.serialize(), widget_id, action_id, rp);
        assert(ctrl);
    }
    catch (std::exception const& e)
    {
        // TODO: log error
        cerr << "perform_action(): " << e.what() << endl;
        try
        {
            // TODO: if things go wrong, we need to make sure that the reply object
            // is disconnected from the middleware, so it gets deallocated.
            reply->finished(ListenerBase::Error, e.what());
            throw;
        }
        catch (...)
        {
            cerr << "perform_action(): unknown exception" << endl;
        }
        throw;
    }
    return ctrl;
}

QueryCtrlProxy ScopeImpl::preview(Result const& result,
                                  ActionMetadata const& hints,
                                  PreviewListenerBase::SPtr const& reply)
{
    if (reply == nullptr)
    {
        throw unity::InvalidArgumentException("Scope::preview(): invalid PreviewListenerBase (nullptr)");
    }

    QueryCtrlProxy ctrl;
    PreviewReplyObject::SPtr ro(make_shared<PreviewReplyObject>(reply, runtime_, to_string()));
    try
    {
        // Create a middleware server-side object that can receive incoming
        // push() and finished() messages over the network.
        MWReplyProxy rp = fwd()->mw_base()->add_reply_object(ro);

        // Forward the search() method across the bus. This is a
        // synchronous twoway interaction with the scope, so it can return
        // the QueryCtrlProxy. Because the Scope implementation has a separate
        // thread for search() calls, this is guaranteed not to block for
        // any length of time. (No application code other than the QueryBase constructor
        // is called by search() on the server side.)
        ctrl = fwd()->preview(result.p->activation_target(), hints.serialize(), rp);
        assert(ctrl);
    }
    catch (std::exception const& e)
    {
        // TODO: log error
        cerr << "preview(): " << e.what() << endl;
        try
        {
            ro->finished(ListenerBase::Error, e.what());
            throw;
        }
        catch (...)
        {
            cerr << "preview(): unknown exception" << endl;
        }
        throw;
    }
    return ctrl;
}

ScopeProxy ScopeImpl::create(MWScopeProxy const& mw_proxy, RuntimeImpl* runtime, std::string const& scope_id)
{
    return make_shared<ScopeImpl>(mw_proxy, runtime, scope_id);
}

MWScopeProxy ScopeImpl::fwd()
{
    return dynamic_pointer_cast<MWScope>(proxy());
}

} // namespace internal

} // namespace scopes

} // namespace unity

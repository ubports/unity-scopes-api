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

using namespace std;

namespace unity
{

namespace scopes
{

namespace internal
{

ScopeImpl::ScopeImpl(MWScopeProxy const& mw_proxy, RuntimeImpl* runtime, std::string const& scope_id) :
    ObjectImpl(mw_proxy, runtime->logger()),
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
    if (reply == nullptr)
    {
        throw unity::InvalidArgumentException("Scope::search(): invalid SearchListenerBase (nullptr)");
    }

    ReplyObject::SPtr ro(make_shared<ResultReplyObject>(reply, runtime_, to_string(), metadata.cardinality(), fwd()->debug_mode()));
    MWReplyProxy rp = fwd()->mw_base()->add_reply_object(ro);

    // "Fake" QueryCtrlProxy that doesn't have a real MWQueryCtrlProxy yet.
    shared_ptr<QueryCtrlImpl> ctrl = make_shared<QueryCtrlImpl>(nullptr, rp, runtime_->logger());

    // We pass a shared pointer to the lambda (instead of the this pointer)
    // to keep ourselves alive until after the lambda fires.
    // Otherwise, if the ScopeProxy for this invocation goes out of scope before the invocation is
    // sent in the new thread, the lambda will call into this by-now-destroyed instance.
    auto impl = dynamic_pointer_cast<ScopeImpl>(shared_from_this());

    auto send_search = [impl, query, metadata, rp, ro, ctrl]() -> void
    {
        try
        {
            // Forward the (synchronous) search() method across the bus.
            auto real_ctrl = dynamic_pointer_cast<QueryCtrlImpl>(impl->fwd()->search(query, metadata.serialize(), rp));
            assert(real_ctrl);

            // Call has completed now, so we update the MWQueryCtrlProxy for the fake proxy
            // with the real proxy that was returned.
            auto new_proxy = dynamic_pointer_cast<MWQueryCtrl>(real_ctrl->proxy());
            assert(new_proxy);
            ctrl->set_proxy(new_proxy);
        }
        catch (std::exception const& e)
        {
            try
            {
                ro->finished(CompletionDetails(CompletionDetails::Error, e.what()));
            }
            catch (...)
            {
            }
        }
    };

    // Send the blocking twoway request asynchronously via the async invocation pool. The waiter thread
    // waits on the future, so it gets cleaned up.
    auto future = runtime_->async_pool()->submit(send_search);
    runtime_->future_queue()->push(move(future));
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

    ReplyObject::SPtr ro(make_shared<ActivationReplyObject>(reply, runtime_, to_string(), fwd()->debug_mode()));
    MWReplyProxy rp = fwd()->mw_base()->add_reply_object(ro);

    shared_ptr<QueryCtrlImpl> ctrl = make_shared<QueryCtrlImpl>(nullptr, rp, runtime_->logger());

    auto impl = dynamic_pointer_cast<ScopeImpl>(shared_from_this());

    auto send_activate = [impl, result, metadata, rp, ro, ctrl]() -> void
    {
        try
        {
            auto real_ctrl = dynamic_pointer_cast<QueryCtrlImpl>(impl->fwd()->activate(result.p->activation_target(),
                                                                                       metadata.serialize(),
                                                                                       rp));
            assert(real_ctrl);

            auto new_proxy = dynamic_pointer_cast<MWQueryCtrl>(real_ctrl->proxy());
            assert(new_proxy);
            ctrl->set_proxy(new_proxy);
        }
        catch (std::exception const& e)
        {
            try
            {
                ro->finished(CompletionDetails(CompletionDetails::Error, e.what()));
            }
            catch (...)
            {
            }
        }
    };

    auto future = runtime_->async_pool()->submit(send_activate);
    runtime_->future_queue()->push(move(future));
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

    ReplyObject::SPtr ro(make_shared<ActivationReplyObject>(reply, runtime_, to_string(), fwd()->debug_mode()));
    MWReplyProxy rp = fwd()->mw_base()->add_reply_object(ro);

    shared_ptr<QueryCtrlImpl> ctrl = make_shared<QueryCtrlImpl>(nullptr, rp, runtime_->logger());

    auto impl = dynamic_pointer_cast<ScopeImpl>(shared_from_this());

    auto send_perform_action = [impl, result, metadata, widget_id, action_id, rp, ro, ctrl]() -> void
    {
        try
        {
            auto real_ctrl = dynamic_pointer_cast<QueryCtrlImpl>(impl->fwd()->perform_action(
                                                                               result.p->activation_target(),
                                                                               metadata.serialize(),
                                                                               widget_id,
                                                                               action_id,
                                                                               rp));
            assert(real_ctrl);

            auto new_proxy = dynamic_pointer_cast<MWQueryCtrl>(real_ctrl->proxy());
            assert(new_proxy);
            ctrl->set_proxy(new_proxy);
        }
        catch (std::exception const& e)
        {
            try
            {
                ro->finished(CompletionDetails(CompletionDetails::Error, e.what()));
            }
            catch (...)
            {
            }
        }
    };

    auto future = runtime_->async_pool()->submit(send_perform_action);
    runtime_->future_queue()->push(move(future));
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

    ReplyObject::SPtr ro(make_shared<PreviewReplyObject>(reply, runtime_, to_string(), fwd()->debug_mode()));
    MWReplyProxy rp = fwd()->mw_base()->add_reply_object(ro);

    shared_ptr<QueryCtrlImpl> ctrl = make_shared<QueryCtrlImpl>(nullptr, rp, runtime_->logger());

    auto impl = dynamic_pointer_cast<ScopeImpl>(shared_from_this());

    auto send_preview = [impl, result, hints, rp, ro, ctrl]() -> void
    {
        try
        {
            auto real_ctrl = dynamic_pointer_cast<QueryCtrlImpl>(impl->fwd()->preview(result.p->activation_target(),
                                                                                      hints.serialize(),
                                                                                      rp));
            assert(real_ctrl);

            auto new_proxy = dynamic_pointer_cast<MWQueryCtrl>(real_ctrl->proxy());
            assert(new_proxy);
            ctrl->set_proxy(new_proxy);
        }
        catch (std::exception const& e)
        {
            try
            {
                ro->finished(CompletionDetails(CompletionDetails::Error, e.what()));
            }
            catch (...)
            {
            }
        }
    };

    auto future = runtime_->async_pool()->submit(send_preview);
    runtime_->future_queue()->push(move(future));
    return ctrl;
}

ChildScopeList ScopeImpl::child_scopes_ordered()
{
    return fwd()->child_scopes_ordered();
}

bool ScopeImpl::set_child_scopes_ordered(ChildScopeList const& child_scopes_ordered)
{
    return fwd()->set_child_scopes_ordered(child_scopes_ordered);
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

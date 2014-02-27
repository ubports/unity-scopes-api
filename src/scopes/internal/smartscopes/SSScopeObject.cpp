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
 * Authored by: Marcus Tomlinson <marcus.tomlinson@canonical.com>
 */

#include <unity/scopes/internal/smartscopes/SSScopeObject.h>
#include <unity/scopes/internal/smartscopes/SmartScope.h>
#include <unity/scopes/internal/smartscopes/SSQueryObject.h>
#include <unity/scopes/internal/MWQuery.h>
#include <unity/scopes/internal/MWReply.h>
#include <unity/scopes/ScopeExceptions.h>
#include <unity/scopes/ActionMetadata.h>
#include <unity/scopes/SearchMetadata.h>
#include <unity/UnityExceptions.h>

namespace unity
{

namespace scopes
{

namespace internal
{

namespace smartscopes
{

SSScopeObject::SSScopeObject(std::string const& ss_scope_id,
                             MiddlewareBase::SPtr middleware,
                             SSRegistryObject::SPtr ss_registry)
    : ss_scope_id_(ss_scope_id)
    , co_(std::make_shared<SSQueryCtrlObject>())
    , qo_(std::make_shared<SSQueryObject>())
    , smartscope_(new SmartScope(ss_registry))
    , ss_registry_(ss_registry)
{
    // Connect the query ctrl to the middleware.
    middleware->add_dflt_query_ctrl_object(co_);

    // Connect the query object to the middleware.
    middleware->add_dflt_query_object(qo_);

    // We tell the ctrl what the query facade is
    co_->set_query(qo_);
}

SSScopeObject::~SSScopeObject() noexcept
{
}

MWQueryCtrlProxy SSScopeObject::search(CannedQuery const& q,
                                             SearchMetadata const& hints,
                                             MWReplyProxy const& reply,
                                             InvokeInfo const& info)
{
    return query(info,
                 reply,
                 [&q, &hints, &info, this ]()->QueryBase::SPtr
                     { return this->smartscope_->search(info.id, q, hints); },
                 [&reply, &hints, this](QueryBase::SPtr query_base)
                     { qo_->add_query(SSQuery::Query, query_base, hints.cardinality(), reply); });
}

MWQueryCtrlProxy SSScopeObject::activate(Result const& result,
                                         ActionMetadata const& hints,
                                         MWReplyProxy const& reply,
                                         InvokeInfo const& info)
{
    return query(info,
                 reply,
                 [&result, &hints, &info, this ]()->QueryBase::SPtr
                     { return this->smartscope_->activate(info.id, result, hints); },
                 [&reply, &info, this](QueryBase::SPtr query_base)
                     { qo_->add_query(SSQuery::Activation, query_base, reply); });
}

MWQueryCtrlProxy SSScopeObject::perform_action(Result const& result,
                                               ActionMetadata const& hints,
                                               std::string const& widget_id,
                                               std::string const& action_id,
                                               MWReplyProxy const& reply,
                                               InvokeInfo const& info)
{
    return query(info,
                 reply,
                 [&result, &hints, &info, &widget_id, &action_id, this ]()->QueryBase::SPtr
                     { return this->smartscope_->perform_action(info.id, result, hints, widget_id, action_id); },
                 [&reply, &info, this](QueryBase::SPtr query_base)
                     { qo_->add_query(SSQuery::Activation, query_base, reply); });
}

MWQueryCtrlProxy SSScopeObject::preview(Result const& result,
                                        ActionMetadata const& hints,
                                        MWReplyProxy const& reply,
                                        InvokeInfo const& info)
{
    return query(info,
                 reply,
                 [&result, &hints, &info, this ]()->QueryBase::SPtr
                     { return this->smartscope_->preview(info.id, result, hints); },
                 [&reply, &info, this](QueryBase::SPtr query_base)
                     { qo_->add_query(SSQuery::Preview, query_base, reply); });
}

MWQueryCtrlProxy SSScopeObject::query(InvokeInfo const& info,
                                      MWReplyProxy const& reply,
                                      std::function<QueryBase::SPtr()> const& query_factory_fun,
                                      std::function<void(QueryBase::SPtr)> const& query_object_fun)
{
    if (!ss_registry_->has_scope(info.id))
    {
        throw ObjectNotExistException("SSScopeObject::query(): Remote scope does not exist", info.id);
    }

    if (!reply)
    {
        // TODO: log error about incoming request containing an invalid reply proxy.
        throw LogicException("SSScopeObject: query() called with null reply proxy");
    }

    // Ask scope to instantiate a new query.
    QueryBase::SPtr query_base;
    try
    {
        query_base = query_factory_fun();
        if (!query_base)
        {
            // TODO: log error, scope returned null pointer.
            throw ResourceException("SmartScope returned nullptr from query_factory_fun()");
        }
    }
    catch (...)
    {
        throw ResourceException("SmartScope threw an exception from query_factory_fun()");
    }

    try
    {
        // add new query to SS query object
        query_object_fun(query_base);

        // Start the query via the middleware (calling run() in a different thread)
        MWQueryProxy query_proxy = info.mw->create_query_proxy(info.id, info.mw->get_query_endpoint());
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
        std::cerr << "query(): " << e.what() << std::endl;
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
        std::cerr << "query(): unknown exception" << std::endl;
        // TODO: log error
        throw;
    }

    return info.mw->create_query_ctrl_proxy(reply->identity() + ".c", info.mw->get_query_ctrl_endpoint());
}

}  // namespace smartscopes

}  // namespace internal

}  // namespace scopes

}  // namespace unity

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
#include <unity/scopes/internal/QueryObject.h>
#include <unity/scopes/internal/MWQuery.h>
#include <unity/scopes/internal/MWReply.h>
#include <unity/scopes/internal/QueryCtrlObject.h>
#include <unity/UnityExceptions.h>

#include <cassert>

namespace unity
{

namespace scopes
{

namespace internal
{

namespace smartscopes
{

SSScopeObject::SSScopeObject(RuntimeImpl* runtime, SSRegistryObject::SPtr registry) :
  runtime_(runtime),
  registry_(registry),
  smartscope_(new SmartScope(registry_))
{
  assert(runtime);
}

SSScopeObject::~SSScopeObject() noexcept
{
}

MWQueryCtrlProxy SSScopeObject::create_query(std::string const& q,
                                             VariantMap const& hints,
                                             MWReplyProxy const& reply,
                                             InvokeInfo const& info)
{
  return query(reply, info.mw,
               [&q, &hints, &info, this]() -> QueryBase::SPtr {
                 return this->smartscope_->create_query(info.id, q, hints);
               },
               [&reply](QueryBase::SPtr query_base, MWQueryCtrlProxy ctrl_proxy) -> QueryObjectBase::SPtr {
                 return std::make_shared<QueryObject>(query_base, reply, ctrl_proxy);
               });
}

MWQueryCtrlProxy SSScopeObject::activate(Result const& result,
                                         VariantMap const& hints,
                                         MWReplyProxy const& reply,
                                         InvokeInfo const& info)
{
  (void)result;
  (void)hints;
  (void)reply;
  (void)info;
  return MWQueryCtrlProxy();
}

MWQueryCtrlProxy SSScopeObject::activate_preview_action(Result const& result,
                                                        VariantMap const& hints,
                                                        std::string const& action_id,
                                                        MWReplyProxy const &reply,
                                                        InvokeInfo const& info)
{
  (void)result;
  (void)hints;
  (void)action_id;
  (void)reply;
  (void)info;
  return MWQueryCtrlProxy();
}

MWQueryCtrlProxy SSScopeObject::preview(Result const& result,
                                        VariantMap const& hints,
                                        MWReplyProxy const& reply,
                                        InvokeInfo const& info)
{
  (void)result;
  (void)hints;
  (void)reply;
  (void)info;
  return MWQueryCtrlProxy();
}

MWQueryCtrlProxy SSScopeObject::query(MWReplyProxy const& reply, MiddlewareBase* mw_base,
                                      std::function<QueryBase::SPtr()> const& query_factory_fun,
                                      std::function<QueryObjectBase::SPtr(QueryBase::SPtr, MWQueryCtrlProxy)> const& query_object_factory_fun)
{
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

  MWQueryCtrlProxy ctrl_proxy;
  try
  {
    // Instantiate the query ctrl and connect it to the middleware.
    QueryCtrlObject::SPtr co(std::make_shared<QueryCtrlObject>());
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

  return ctrl_proxy;
}

} // namespace smartscopes

} // namespace internal

} // namespace scopes

} // namespace unity

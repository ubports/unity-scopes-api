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

SSScopeObject::SSScopeObject(MiddlewareBase::SPtr middleware, SSRegistryObject::SPtr registry) :
  co_(std::make_shared<QueryCtrlObject>()),
  qo_(std::make_shared<SSQueryObject>()),
  smartscope_(new SmartScope(registry))
{
  // Connect the query ctrl to the middleware.
  middleware->add_dflt_query_ctrl_object(co_);

  // Connect the query object to the middleware.
  middleware->add_dflt_query_object(qo_);

  // We tell the ctrl what the query facade is
  co_->set_query(qo_);

  // We pass a shared_ptr to the qo to the qo itself, so the qo can hold the reference
  // count high until the run() request arrives in the query via the middleware.
  qo_->set_self(qo_);
}

SSScopeObject::~SSScopeObject() noexcept
{
}

MWQueryCtrlProxy SSScopeObject::create_query(std::string const& q,
                                             VariantMap const& hints,
                                             MWReplyProxy const& reply,
                                             InvokeInfo const& info)
{
  return query(reply,
               [&q, &hints, &info, this]() -> QueryBase::SPtr {
                 return this->smartscope_->create_query(info.id, q, hints);
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

MWQueryCtrlProxy SSScopeObject::query(MWReplyProxy const& reply,
                                      std::function<QueryBase::SPtr()> const& query_factory_fun)
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

  try
  {
    ///! add query base and reply to QO
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

  return MWQueryCtrlProxy();
}

} // namespace smartscopes

} // namespace internal

} // namespace scopes

} // namespace unity

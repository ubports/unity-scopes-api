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
 * Authored by: Pawel Stolowski <pawel.stolowski@canonical.com>
*/

#include <scopes/internal/ActivationQueryObject.h>
#include <scopes/ListenerBase.h>
#include <scopes/ActivationBase.h>
#include <scopes/internal/MWReply.h>
#include <iostream>

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

ActivationQueryObject::ActivationQueryObject(std::shared_ptr<ActivationBase> const& act_base, MWReplyProxy const& reply, MWQueryCtrlProxy const& ctrl)
    : QueryObjectBase(),
    act_base_(act_base),
    reply_(reply)
{
}

void ActivationQueryObject::cancel()
{
    //TODO
}

void ActivationQueryObject::run(MWReplyProxy const& reply) noexcept
{
    try
    {
        // no need for intermediate proxy (like with ReplyImpl::create),
        // since we get single return value from the public API
        // and just push it ourseleves
        auto res = act_base_->activate();
        reply->push(res.serialize());
    }
    catch (std::exception const& e)
    {
        // TODO: log error
        reply_->finished(ListenerBase::Error, e.what());     // Oneway, can't block
        cerr << "ActivationQueryObject::run(): " << e.what() << endl;
    }
    catch (...)
    {
        // TODO: log error
        reply_->finished(ListenerBase::Error, "unknown exception");     // Oneway, can't block
        cerr << "ActivationQueryObject::run(): unknown exception" << endl;
    }
}

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

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

#include <scopes/internal/zmq_middleware/QueryCtrlI.h>

#include <internal/zmq_middleware/capnproto/QueryCtrl.capnp.h>
#include <scopes/internal/zmq_middleware/ObjectAdapter.h>
#include <scopes/internal/QueryCtrlObject.h>

#include <cassert>

using namespace std;

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

namespace zmq_middleware
{

/*

interface QueryCtrl
{
    void cancel();
    void destroy();
};

*/

using namespace std::placeholders;

QueryCtrlI::QueryCtrlI(QueryCtrlObject::SPtr const& qo) :
    ServantBase(qo, { { "cancel", bind(&QueryCtrlI::cancel_, this, _1, _2, _3) },
                      { "destroy", bind(&QueryCtrlI::destroy_, this, _1, _2, _3) } })

{
}

QueryCtrlI::~QueryCtrlI() noexcept
{
}

void QueryCtrlI::cancel_(Current const&,
                         capnp::ObjectPointer::Reader&,
                         capnproto::Response::Builder&)
{
    auto delegate = dynamic_pointer_cast<QueryCtrlObject>(del());
    delegate->cancel();
}

void QueryCtrlI::destroy_(Current const&,
                          capnp::ObjectPointer::Reader&,
                          capnproto::Response::Builder&)
{
    auto delegate = dynamic_pointer_cast<QueryCtrlObject>(del());
    delegate->destroy();
}

} // namespace zmq_middleware

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

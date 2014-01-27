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

#include <unity/scopes/internal/zmq_middleware/QueryCtrlI.h>

#include <scopes/internal/zmq_middleware/capnproto/QueryCtrl.capnp.h>

#include <cassert>

using namespace std;

namespace unity
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

QueryCtrlI::QueryCtrlI(QueryCtrlObjectBase::SPtr const& qo) :
    ServantBase(qo, { { "cancel", bind(&QueryCtrlI::cancel_, this, _1, _2, _3) },
                      { "destroy", bind(&QueryCtrlI::destroy_, this, _1, _2, _3) } })

{
}

QueryCtrlI::~QueryCtrlI()
{
}

void QueryCtrlI::cancel_(Current const& current,
                         capnp::AnyPointer::Reader&,
                         capnproto::Response::Builder&)
{
    auto delegate = dynamic_pointer_cast<QueryCtrlObjectBase>(del());
    delegate->cancel(to_info(current));
}

void QueryCtrlI::destroy_(Current const& current,
                          capnp::AnyPointer::Reader&,
                          capnproto::Response::Builder&)
{
    auto delegate = dynamic_pointer_cast<QueryCtrlObjectBase>(del());
    delegate->destroy(to_info(current));
}

} // namespace zmq_middleware

} // namespace internal

} // namespace scopes

} // namespace unity

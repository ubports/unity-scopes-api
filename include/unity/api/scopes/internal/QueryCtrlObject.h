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

#ifndef UNITY_API_REPLYS_INTERNAL_QUERYCTRLOBJECT_H
#define UNITY_API_REPLYS_INTERNAL_QUERYCTRLOBJECT_H

#include <unity/api/scopes/internal/AbstractObject.h>

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

class QueryObject;

// A QueryCtrlObject sits in between the incoming requests from the middleware layer and the
// QueryCtrlBase-derived implementation. This allows us to receive cancel requests. In turn,
// the implementation of this object ensures that the corresponding ReplyObject is disabled.
// TODO: Probably need to flesh out this comment.

class QueryCtrlObject final : public AbstractObject
{
public:
    UNITY_DEFINES_PTRS(QueryCtrlObject);

    QueryCtrlObject();
    virtual ~QueryCtrlObject() noexcept;

    // Remote operation implementations
    void cancel();
    void destroy();

    // Called by create_query() after instantiation to tell this ctrl what its corresponding query proxy
    // and query facade are.
    void set_query(std::shared_ptr<QueryObject> const& qo);

private:
    std::weak_ptr<QueryObject> qo_;
};

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

#endif

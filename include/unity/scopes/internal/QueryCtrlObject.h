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

#ifndef UNITY_SCOPES_INTERNAL_QUERYCTRLOBJECT_H
#define UNITY_SCOPES_INTERNAL_QUERYCTRLOBJECT_H

#include <unity/scopes/internal/QueryCtrlObjectBase.h>

#include <atomic>

namespace unity
{

namespace scopes
{

namespace internal
{

class QueryObjectBase;

class QueryCtrlObject final : public QueryCtrlObjectBase
{
public:
    UNITY_DEFINES_PTRS(QueryCtrlObject);

    QueryCtrlObject();
    virtual ~QueryCtrlObject();

    // Remote operation implementations
    virtual void cancel(InvokeInfo const& info) override;
    virtual void destroy(InvokeInfo const& info) override;

    // Called by create_query() after instantiation to tell this ctrl what its corresponding
    // query facade is.
    void set_query(std::shared_ptr<QueryObjectBase> const& qo);

private:
    std::weak_ptr<QueryObjectBase> qo_;
    std::atomic_bool destroyed_;
};

} // namespace internal

} // namespace scopes

} // namespace unity

#endif

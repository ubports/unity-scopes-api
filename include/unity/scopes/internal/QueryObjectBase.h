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

#pragma once

#include <unity/scopes/internal/AbstractObject.h>
#include <unity/scopes/internal/MWReplyProxyFwd.h>
#include <unity/util/DefinesPtrs.h>

namespace unity
{

namespace scopes
{

namespace internal
{

struct InvokeInfo;

class QueryObjectBase : public AbstractObject
{
public:
    UNITY_DEFINES_PTRS(QueryObjectBase);
    // Remote operation implementation
    virtual void run(MWReplyProxy const& reply, InvokeInfo const& info) noexcept = 0;

    virtual void cancel(InvokeInfo const& info) = 0;                    // Called locally, by QueryCtrlObject
    virtual bool pushable(InvokeInfo const& info) const noexcept = 0;   // Called locally, by ReplyImpl
    virtual int cardinality(InvokeInfo const& info) const noexcept = 0; // Called locally, by ReplyImpl

    // Used to hold the reference count high until the run call arrives via the middleware,
    // and we can pass the shared_ptr to the ReplyImpl.
    virtual void set_self(SPtr const& self) noexcept = 0;
};

} // namespace internal

} // namespace scopes

} // namespace unity

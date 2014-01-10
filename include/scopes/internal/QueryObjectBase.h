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

#ifndef UNITY_INTERNAL_QUERYOBJECTBASE_H
#define UNITY_INTERNAL_QUERYOBJECTBASE_H

#include <scopes/internal/AbstractObject.h>
#include <unity/util/DefinesPtrs.h>
#include <scopes/internal/MWReplyProxyFwd.h>

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

class QueryObjectBase : public AbstractObject
{
public:
    UNITY_DEFINES_PTRS(QueryObjectBase);
    // Remote operation implementation
    virtual void run(MWReplyProxy const& reply) noexcept = 0;
    virtual void cancel() = 0;
};

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

#endif

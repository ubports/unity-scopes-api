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

#ifndef UNITY_API_SCOPES_INTERNAL_ICEMIDDLEWARE_QUERYI_H
#define UNITY_API_SCOPES_INTERNAL_ICEMIDDLEWARE_QUERYI_H

#include <unity/api/scopes/internal/ice_middleware/slice/Query.h>

#include <memory>

namespace unity
{

namespace api
{

namespace scopes
{

class ScopeBase;

namespace internal
{

class QueryObject;

namespace ice_middleware
{

class IceMiddleware;

class QueryI : public middleware::Query
{
public:
    QueryI(IceMiddleware* mw_base, std::shared_ptr<QueryObject> const& co);
    virtual ~QueryI() noexcept;

    // Slice operations
    virtual void run(middleware::ReplyPrx const& r, Ice::Current const&) override;

private:
    void destroy(Ice::Current const&);

    IceMiddleware* mw_base_;
    std::shared_ptr<QueryObject> qo_;
};

typedef IceUtil::Handle<QueryI> QueryIPtr;

} // namespace ice_middleware

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

#endif

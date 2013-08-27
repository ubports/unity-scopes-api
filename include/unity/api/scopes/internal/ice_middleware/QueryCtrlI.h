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

#ifndef UNITY_API_SCOPES_INTERNAL_ICEMIDDLEWARE_QUERYCTRLI_H
#define UNITY_API_SCOPES_INTERNAL_ICEMIDDLEWARE_QUERYCTRLI_H

#include <slice/unity/api/scopes/internal/ice_middleware/QueryCtrl.h>

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

class QueryCtrlObject;

namespace ice_middleware
{

class QueryCtrlI : public middleware::QueryCtrl
{
public:
    QueryCtrlI(std::shared_ptr<QueryCtrlObject> const& co);
    virtual ~QueryCtrlI();

    virtual void cancel(Ice::Current const&) override;

private:
    void destroy(Ice::Current const&) noexcept;

    std::shared_ptr<QueryCtrlObject> co_;
};

typedef IceUtil::Handle<QueryCtrlI> QueryCtrlIPtr;

} // namespace ice_middleware

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

#endif

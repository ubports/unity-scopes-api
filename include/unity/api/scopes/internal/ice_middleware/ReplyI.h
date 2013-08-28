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

#ifndef UNITY_API_SCOPES_INTERNAL_ICEMIDDLEWARE_REPLYI_H
#define UNITY_API_SCOPES_INTERNAL_ICEMIDDLEWARE_REPLYI_H

#include <unity/api/scopes/internal/ice_middleware/slice/Reply.h>
#include <unity/api/scopes/internal/ReplyObject.h>

namespace unity
{

namespace api
{

namespace scopes
{

class ScopeBase;

namespace internal
{

namespace ice_middleware
{

class ReplyI : public middleware::Reply
{
public:
    ReplyI(ReplyObject::SPtr const& ro);

    virtual void push(std::string const& result, Ice::Current const&) override;
    virtual void finished(Ice::Current const&) override;

private:
    ReplyObject::SPtr ro_;
};

typedef IceUtil::Handle<ReplyI> ReplyIPtr;

} // namespace ice_middleware

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

#endif

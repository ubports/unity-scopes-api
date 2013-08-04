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

#include <unity/api/scopes/internal/ScopeObject.h>

#include <unity/api/scopes/ScopeBase.h>
#include <unity/Exception.h>

#include <cassert>

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

ScopeObject::ScopeObject(ScopeBase* scope_base) :
    scope_base_(scope_base)
{
    assert(scope_base);
}

ScopeObject::~ScopeObject() noexcept
{
}

void ScopeObject::query(std::string const& q, ReplyProxy::SPtr const& reply)
{
    if (!reply)
    {
        // We can't assert here because the null proxy may have been sent by a broken client, that is,
        // it can be null because it was sent by the remote end as null.

        // TODO: log error about incoming request containing an invalid reply proxy.
        return;
    }

    // If the query() method of the scope throws, we make sure that we call finished() on the reply,
    // so the originator of the query gets its finished() method invoked.
    try
    {
        scope_base_->query(q, reply);
    }
    catch (unity::Exception const& e)
    {
        try
        {
            reply->finished();
        }
        catch (...)
        {
        }
        // TODO: log error
    }
    catch (...)
    {
        try
        {
            reply->finished();
        }
        catch (...)
        {
        }
        // TODO: log error
    }
}

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

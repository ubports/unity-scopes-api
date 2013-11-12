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

#include <scopes/internal/MiddlewareBase.h>
#include <scopes/internal/MWReply.h>
#include <scopes/internal/RuntimeImpl.h>
#include <scopes/internal/ResultItemImpl.h>
#include <scopes/internal/ReplyImpl.h>
#include <scopes/ResultItem.h>
#include <scopes/ScopeExceptions.h>
#include <scopes/Reply.h>

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

ReplyImpl::ReplyImpl(MWReplyProxy const& mw_proxy, std::shared_ptr<QueryObject> const& qo) :
    mw_proxy_(mw_proxy),
    qo_(qo),
    finished_(false)
{
    assert(mw_proxy);
    assert(qo);
}

ReplyImpl::~ReplyImpl() noexcept
{
    try
    {
        finished();
    }
    catch (...)
    {
        // TODO: log error
    }
}

bool ReplyImpl::push(unity::api::scopes::ResultItem const& result)
{
    if (!finished_.load())
    {
        try
        {
            mw_proxy_->push(result.p->to_variant_map());
            return true;
        }
        catch (MiddlewareException const& e)
        {
            // TODO: log error
            finished();
            return false;
        }
    }
    return false;
}

void ReplyImpl::finished()
{
    if (!finished_.exchange(true))
    {
        try
        {
            mw_proxy_->finished();
        }
        catch (MiddlewareException const& e)
        {
            // TODO: log error
        }
    }
}

ReplyProxy ReplyImpl::create(MWReplyProxy const& mw_proxy, std::shared_ptr<QueryObject> const& qo)
{
    return ReplyProxy(new Reply(new ReplyImpl(mw_proxy, qo)));
}

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

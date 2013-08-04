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

#include <unity/api/scopes/internal/ReplyProxyImpl.h>

#include <unity/api/scopes/internal/MiddlewareBase.h>
#include <unity/api/scopes/internal/MWReplyProxy.h>
#include <unity/api/scopes/internal/RuntimeImpl.h>
#include <unity/api/scopes/ScopeExceptions.h>

using namespace std;

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

ReplyProxyImpl::ReplyProxyImpl(MWReplyProxy::SPtr const& mw_proxy) :
    mw_proxy_(mw_proxy),
    finished_(false)
{
}

ReplyProxyImpl::~ReplyProxyImpl() noexcept
{
    if (!finished_.load())
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
}

void ReplyProxyImpl::send(std::string const& result)
{
    if (!finished_.load())
    {
        try
        {
            return mw_proxy_->send(result);
        }
        catch (MiddlewareException const& e)
        {
            // TODO: log error
            finished();
        }
    }
}

void ReplyProxyImpl::finished()
{
    if (!finished_.exchange(true))
    {
        try
        {
            return mw_proxy_->finished();
        }
        catch (MiddlewareException const& e)
        {
            // TODO: log error
        }
    }
}

ReplyProxy::SPtr ReplyProxyImpl::create(MWReplyProxy::SPtr const& mw_proxy)
{
    return ReplyProxy::SPtr(new ReplyProxy(new ReplyProxyImpl(mw_proxy)));
}

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

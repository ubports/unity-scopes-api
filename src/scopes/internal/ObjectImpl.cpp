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

#include <unity/scopes/internal/ObjectImpl.h>

#include <unity/scopes/internal/MWObjectProxy.h>
#include <unity/scopes/ScopeExceptions.h>

#include <cassert>

using namespace std;
using namespace unity::scopes;

namespace unity
{

namespace scopes
{

namespace internal
{

ObjectImpl::ObjectImpl(MWProxy const& mw_proxy, boost::log::sources::severity_channel_logger_mt<>& logger)
    : logger_(logger_)
{
    lock_guard<mutex> lock(proxy_mutex_); // TODO: Why this lock?
    mw_proxy_ = mw_proxy;
}

ObjectImpl::~ObjectImpl()
{
}

string ObjectImpl::identity()
{
    check_proxy();
    return mw_proxy_->identity();
}

string ObjectImpl::target_category()
{
    return mw_proxy_->target_category();
}

string ObjectImpl::endpoint()
{
    check_proxy();
    return mw_proxy_->endpoint();
}

int64_t ObjectImpl::timeout()
{
    check_proxy();
    return mw_proxy_->timeout();
}

string ObjectImpl::to_string()
{
    check_proxy();
    return mw_proxy_->to_string();
}

void ObjectImpl::ping()
{
    check_proxy();
    mw_proxy_->ping();
}

MWProxy ObjectImpl::proxy()
{
    lock_guard<mutex> lock(proxy_mutex_);
    return mw_proxy_;
}

void ObjectImpl::set_proxy(MWProxy const& p)
{
    assert(p);
    lock_guard<mutex> lock(proxy_mutex_);
    assert(!mw_proxy_);
    mw_proxy_ = p;
}

void ObjectImpl::check_proxy()
{
    lock_guard<mutex> lock(proxy_mutex_);
    if (!mw_proxy_)
    {
        throw MiddlewareException("Cannot invoke on null proxy");
    }
}

} // namespace internal

} // namespace scopes

} // namespace unity

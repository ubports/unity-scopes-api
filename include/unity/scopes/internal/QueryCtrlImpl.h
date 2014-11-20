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

#ifndef UNITY_SCOPES_INTERNAL_QUERYCTRLIMPL_H
#define UNITY_SCOPES_INTERNAL_QUERYCTRLIMPL_H

#include <unity/scopes/internal/MWQueryCtrlProxyFwd.h>
#include <unity/scopes/internal/MWReplyProxyFwd.h>
#include <unity/scopes/internal/ObjectImpl.h>
#include <unity/scopes/QueryCtrl.h>

namespace unity
{

namespace scopes
{

namespace internal
{

class ScopeImpl;

class QueryCtrlImpl : public virtual unity::scopes::QueryCtrl, public virtual ObjectImpl
{
public:
    QueryCtrlImpl(MWQueryCtrlProxy const& ctrl_proxy,
                  MWReplyProxy const& reply_proxy,
                  boost::log::sources::severity_channel_logger_mt<>& logger);
    virtual ~QueryCtrlImpl();

    virtual void cancel() override;

    void set_proxy(MWQueryCtrlProxy const& p);

private:
    MWQueryCtrlProxy fwd();

    MWReplyProxy reply_proxy_;

    bool ready_;                // True once ObjectImpl::set_proxy() was called
    bool cancelled_;            // True if cancel() is called before set_proxy() was called
    std::mutex mutex_;          // Protects ready_ and cancelled_

    friend class ScopeImpl;     // Allows ScopeImpl to call set_proxy()
};

} // namespace internal

} // namespace scopes

} // namespace unity

#endif

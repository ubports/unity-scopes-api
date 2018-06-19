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

#pragma once

#include <unity/scopes/internal/MWReplyProxyFwd.h>
#include <unity/scopes/internal/ObjectImpl.h>
#include <unity/scopes/ListenerBase.h>
#include <unity/scopes/Reply.h>
#include <unity/scopes/SearchReplyProxyFwd.h>
#include <unity/scopes/Variant.h>

#include <atomic>
#include <functional>

namespace unity
{

namespace scopes
{

namespace internal
{

class QueryObjectBase;

class ReplyImpl : public virtual unity::scopes::Reply, public virtual ObjectImpl
{
public:
    ReplyImpl(MWReplyProxy const& mw_proxy, std::shared_ptr<QueryObjectBase>const & qo);
    virtual ~ReplyImpl();

    virtual void finished() override;
    virtual void error(std::exception_ptr ex) override;
    virtual void info(OperationInfo const& op_info) override;

    typedef std::function<void()> CleanupFunc;

protected:
    bool push(VariantMap const& variant_map);

    MWReplyProxy fwd();

private:
    std::shared_ptr<QueryObjectBase> qo_;
    std::atomic_bool finished_;
};

} // namespace internal

} // namespace scopes

} // namespace unity

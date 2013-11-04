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

#ifndef UNITY_INTERNAL_REPLYIMPL_H
#define UNITY_INTERNAL_REPLYIMPL_H

#include <unity/api/scopes/internal/MWReplyProxyFwd.h>
#include <unity/api/scopes/ReplyProxyFwd.h>

#include <atomic>

namespace unity
{

namespace api
{

namespace scopes
{

class ResultItem;

namespace internal
{

class QueryObject;

// Proxy used by a scope to push results for a query.
// To indicate that it has sent the last result, the scope must call finished().
// Subsequent calls to finished() are ignored.
// Calls to push() after finished() was called are ignored.
// If the proxy goes out of scope before finished was called, it implicitly calls finished().

class ReplyImpl final
{
public:
    ReplyImpl(MWReplyProxy const& mw_proxy, std::shared_ptr<QueryObject>const & qo);
    ~ReplyImpl() noexcept;

    bool push(unity::api::scopes::ResultItem const& result);
    void finished();

    static ReplyProxy create(MWReplyProxy const& mw_proxy, std::shared_ptr<QueryObject> const& qo);

    typedef std::function<void()> CleanupFunc;

private:
    MWReplyProxy mw_proxy_;
    std::shared_ptr<QueryObject> qo_;
    std::atomic_bool finished_;
};

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

#endif

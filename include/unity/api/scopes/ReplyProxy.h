/*
 * Copyright (C) 2013 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the Lesser GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * Lesser GNU General Public License for more details.
 *
 * You should have received a copy of the Lesser GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Michi Henning <michi.henning@canonical.com>
 */

#ifndef UNITY_API_SCOPES_REPLYPROXY_H
#define UNITY_API_SCOPES_REPLYPROXY_H

#include <unity/api/scopes/ObjectProxy.h>

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{
class ReplyProxyImpl;
}

/**
\brief ReplyProxy allows the results of a query to be sent to the source of the query.
*/

class UNITY_API ReplyProxy : public ObjectProxy
{
public:
    /// @cond
    UNITY_DEFINES_PTRS(ReplyProxy);
    /// @endcond

    /**
    \brief Sends a single result to the source of a query.
    Any calls to send() after finished() was called are ignored.
    */
    virtual void send(std::string const& result);

    /**
    \brief Informs the source of a query that the query results are complete.
    Calling finished() informs the source of a query that the final result for the query
    was sent, that is, that the query is complete.
    The scope application code is responsible for calling finished() once it has sent the
    final result for a query.
    Multiple calls to finished() are ignored.
    The destructor implicitly calls finished() if a ReplyProxy goes out of scope without
    a prior call to finished().
    */
    virtual void finished();

    /**
    \brief Destroys a ReplyProxy.
    If a ReplyProxy goes out of scope without a prior call to finished(), the destructor implicitly calls finished().
    */
    virtual ~ReplyProxy() noexcept;

private:
    ReplyProxy(internal::ReplyProxyImpl* impl);         // Only instantiable by ReplyProxyImpl
    friend class internal::ReplyProxyImpl;

    std::shared_ptr<internal::ReplyProxyImpl> p;
};

} // namespace scopes

} // namespace api

} // namespace unity

#endif

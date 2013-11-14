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

#ifndef UNITY_API_SCOPES_REPLY_H
#define UNITY_API_SCOPES_REPLY_H

#include <scopes/ObjectProxy.h>
#include <scopes/ReplyProxyFwd.h>
#include <scopes/Category.h>

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{
class QueryObject;
class ReplyImpl;
}

class ResultItem;

/**
\brief Reply allows the results of a query to be sent to the source of the query.
*/

class UNITY_API Reply : public ObjectProxy
{
public:
    Reply(Reply const&) = default;

    Category::SCPtr add_category(std::string const &id, std::string const &renderer);
    Category::SCPtr find_category(std::string const& id) const;

    // TODO: document return value from push()
    /**
    \brief Sends a single result to the source of a query.
    Any calls to push() after finished() was called are ignored.
    \return The return value is true if the result was accepted, false otherwise.
    A false return value is due to either finished() having been called earlier,
    or the client that sent the query having cancelled that query.
    */
    bool push(ResultItem const& result) const;

    /**
    \brief Informs the source of a query that the query results are complete.
    Calling finished() informs the source of a query that the final result for the query
    was sent, that is, that the query is complete.
    The scope application code is responsible for calling finished() once it has sent the
    final result for a query.
    Multiple calls to finished() are ignored.
    The destructor implicitly calls finished() if a Reply goes out of scope without
    a prior call to finished().
    */
    void finished() const;

    /**
    \brief Destroys a Reply.
    If a Reply goes out of scope without a prior call to finished(), the destructor implicitly calls finished().
    */
    virtual ~Reply() noexcept;

private:
    Reply(internal::ReplyImpl* impl);         // Instantiable only by ReplyImpl
    friend class internal::ReplyImpl;

    std::shared_ptr<internal::ReplyImpl> p;
    std::shared_ptr<internal::QueryObject> qo;  // Used to reference
};

} // namespace scopes

} // namespace api

} // namespace unity

#endif

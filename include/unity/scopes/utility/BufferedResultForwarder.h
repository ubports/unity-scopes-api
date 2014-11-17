/*
 * Copyright (C) 2014 Canonical Ltd
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
 * Authored by: Pawel Stolowski <pawel.stolowski@canonical.com>
 */

#pragma once

#include <unity/scopes/SearchListenerBase.h>
#include <unity/scopes/SearchReplyProxyFwd.h>

namespace unity
{

namespace scopes
{

namespace experimental
{
    class Annotation;
}

namespace internal
{
    class BufferedResultForwarderImpl;
}

namespace utility
{

class BufferedResultForwarder : public unity::scopes::SearchListenerBase
{
public:
    /// @cond
    NONCOPYABLE(BufferedResultForwarder);
    UNITY_DEFINES_PTRS(BufferedResultForwarder);
    /// @endcond

    /**
    \brief Create BufferedResultForwarder that forwards results to given upstream reply proxy.
    */
    BufferedResultForwarder(unity::scopes::SearchReplyProxy const& upstream);

    /**
    \brief Create BufferedResultForwarder that forwards results to given upstream reply proxy before another forwarder.

    Create BufferedResultForwarder and chain them with this constructor to define the desired order of categories.
    */
    BufferedResultForwarder(unity::scopes::SearchReplyProxy const& upstream, BufferedResultForwarder::SPtr const& next_forwarder);

    /**
    \brief Default implementation of result forwarding.

    The default implementation just forwards incoming results unchanged to the upstream reply proxy, and marks this BufferedResultForwarder
    ready by calling set_ready() after forwarding first result. This makes it work fine when aggregating from a child scope which only has
    one category, as this is enough to ensure proper order of categories from aggregating scope. When aggregating child scope which
    creates multiple categories, reimplement this method and call set_ready() as soon as your push() handler isn't going to introduce any new categories.

    This method is called once by the scopes run time for each result that is returned by a query().

    \param result The received result
    */
    void push(CategorisedResult result) override;

    /**
    \brief Check if this BufferedResultForwarder is ready and results can be pushed directly to the upstream proxy.

    \return true if this forwarder called set_ready().
     */
    bool is_ready() const;

protected:
    /**
     \brief Mark this forwarder as "ready".

     This method should be called from custom implementation of BufferedResultForwarder as soon as it's not going to introduce any new
     categories.
     */
    void set_ready();

    /**
    \brief Get upstream proxy for pushing data to.

    Returns an instance of buffered reply proxy for all push, register_departments and register_category operations.

    \return buffered reply proxy
    */
    unity::scopes::SearchReplyProxy const& upstream();

    void finished(CompletionDetails const& details) override;

private:
    friend class internal::BufferedResultForwarderImpl;

    internal::BufferedResultForwarderImpl *p;
};

} // namespace utility

} // namespace scopes

} // namespace unity

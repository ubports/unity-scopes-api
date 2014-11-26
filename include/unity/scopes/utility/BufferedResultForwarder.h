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

namespace utility
{

namespace internal
{
    class BufferedResultForwarderImpl;
}

class BufferedResultForwarder : public unity::scopes::SearchListenerBase
{
public:
    /// @cond
    NONCOPYABLE(BufferedResultForwarder);
    UNITY_DEFINES_PTRS(BufferedResultForwarder);
    /// @endcond

    /**
    \brief Create a forwarder that sends results to to its upstream reply proxy.

    \param upstream The reply proxy for the upstream receiver.
    */
    BufferedResultForwarder(unity::scopes::SearchReplyProxy const& upstream);

    /**
    \brief Create a forwarder that sends (at least one) result to its upstream
    reply proxy before indicating that it is ready to its follower.

    \param upstream The reply proxy for the upstream receiver.
    \param next_forwarder The forwarder that becomes ready once this forwarder calls set_ready().
    */
    BufferedResultForwarder(unity::scopes::SearchReplyProxy const& upstream, BufferedResultForwarder::SPtr const& next_forwarder);

    /**
    \brief Forwards a single result before calling `set_ready()`.

    This default implementation forwards incoming results unchanged to the upstream reply proxy and
    and marks the forwarder ready after forwarding the first result.

    This method is called once by the scopes run time for each result that is returned by a query().

    \param result The received result
    */
    void push(CategorisedResult result) override;

    /**
    \brief Check if this forwarder is ready.

    Once ready, the forwarder no longer buffers any results and passes them to the upstream proxy immediately.

    \return `true` if this forwarder called set_ready(), `false` otherwise.
     */
    bool is_ready() const;

protected:
    /**
    \brief Mark this forwarder as ready.

    If you create a custom forwarder, you should call this method as soon as your forwarder will
    no longer push results for new categories.
    */
    void set_ready();

    /**
    \brief Get the upstream proxy.

    Returns an instance of buffered reply proxy for all push, register_departments, and register_category operations.
    Note that this proxy is _not_ the same proxy as the one passed to the constructor.

    \return The buffered reply proxy.
    */
    unity::scopes::SearchReplyProxy upstream() const;

    void finished(CompletionDetails const& details) override;

private:
    friend class internal::BufferedResultForwarderImpl;

    internal::BufferedResultForwarderImpl *p;
};

} // namespace utility

} // namespace scopes

} // namespace unity

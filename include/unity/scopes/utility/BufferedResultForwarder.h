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

/*
   auto scope1fwd = make_shared<BufferedResultForwarder>(upstream_reply);
   auto scope2fwd = make_shared<BufferedResultForwarder>(upstream_reply, scope1fwd); // force order

   subsearch(scope1proxy, "", "", FilterState(), metadata, scope1fwd);
   subsearch(scope2proxy, "", "", FilterState(), metadata, scope2fwd);
 */

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

    BufferedResultForwarder(unity::scopes::SearchReplyProxy const& upstream);
    BufferedResultForwarder(unity::scopes::SearchReplyProxy const& upstream, BufferedResultForwarder::SPtr const& next_forwarder);

    bool is_ready() const;

protected:
    void set_ready();
    unity::scopes::SearchReplyProxy const& upstream();

    void finished(CompletionDetails const& details) override;

private:
    friend class internal::BufferedResultForwarderImpl;

    internal::BufferedResultForwarderImpl *p;
};

} // namespace utility

} // namespace scopes

} // namespace unity

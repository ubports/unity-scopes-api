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

#include <unity/scopes/SearchReplyProxyFwd.h>
#include <unity/scopes/utility/BufferedResultForwarder.h>

#include <vector>
#include <memory>

namespace unity
{

namespace scopes
{

namespace internal
{

class BufferedResultForwarderImpl
{
public:
    BufferedResultForwarderImpl(unity::scopes::SearchReplyProxy const& upstream);
    BufferedResultForwarderImpl(unity::scopes::SearchReplyProxy const& upstream, unity::scopes::utility::BufferedResultForwarder::SPtr const& next_forwarder);

    unity::scopes::SearchReplyProxy const& upstream();
    bool is_ready() const;
    void set_ready();
    void notify_ready();
    void flush_and_notify();
    void finished(CompletionDetails const& details);

private:
    bool ready_;
    bool previous_ready_;
    unity::scopes::SearchReplyProxy const upstream_;
    std::weak_ptr<utility::BufferedResultForwarder> next_;
};

} // namespace internal

} // namespace scopes

} // namespace unity

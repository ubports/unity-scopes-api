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

#include <unity/scopes/utility/BufferedResultForwarder.h>
#include <unity/scopes/internal/BufferedResultForwarderImpl.h>

namespace unity
{

namespace scopes
{

namespace utility
{

BufferedResultForwarder::BufferedResultForwarder(unity::scopes::SearchReplyProxy const& upstream)
    : p(new internal::BufferedResultForwarderImpl(upstream))
{
}

BufferedResultForwarder::BufferedResultForwarder(unity::scopes::SearchReplyProxy const& upstream, BufferedResultForwarder::SPtr const& next_forwarder)
    : p(new internal::BufferedResultForwarderImpl(upstream, next_forwarder))
{
}

bool BufferedResultForwarder::is_ready() const
{
    return p->is_ready();
}

void BufferedResultForwarder::set_ready()
{
    p->set_ready();
}

void BufferedResultForwarder::push(CategorisedResult result)
{
    p->push(result);
}

unity::scopes::SearchReplyProxy const& BufferedResultForwarder::upstream()
{
    return p->upstream();
}

void BufferedResultForwarder::finished(CompletionDetails const& details)
{
    p->finished(details);
}

}

}

}


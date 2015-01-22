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
#include <unity/scopes/utility/internal/BufferedResultForwarderImpl.h>

namespace unity
{

namespace scopes
{

namespace utility
{

/*!
\class BufferedResultForwarder

\brief Base class for a client to receive and buffer the results of a query until another
BufferedResultForwarder becomes ready.

This class implements result buffering, useful for aggregator scopes that receive
results from multiple child scopes and need to display categories in a specific order.
The aggregator scope must create an instance of BufferedResultForwarder
for every child scope it queries and chain the instances together in the desired order.

The default implementation of BufferedResultForwarder forwards the results it receives upstream
and declares itself "ready" after receiving the first result. The results are then buffered until
all prior forwarders have declared themselves ready. Buffering is very efficient&mdash;results are buffered
only until proper order is guaranteed, and buffering is disabled for forwarders that do not need
to wait for a predecessor to become ready. This means that results are pushed to the client
(displayed) as early as possible.

The default implementation only pushes results and their categories, but ignores departments, filters,
and annotations. If you wish to handle and forward these, you must to derive your own forwarder
from BufferedResultForwarder and override the appropropriate methods of the
\link unity::scopes::SearchListenerBase SearchListenerBase\endlink class.

The default implementation buffers a single result before indicating to its follower that it is ready.
This means that the first category from each child determines overall order. For example, if each
child produces results for a single category, the chaining insures the correct order (results from child A
followed by results from child B, or vice versa). However, if child A produces results for categories A1
and A2, and child B produces results for categories B1 and B2, the overall order could, for example, be
A1, B1, A2, B2, or it could be A1, A2, B1, B2, or it could be A1, B2, B1, A2 (among others).

If you want to ensure that all categories from child A arrive in a particular order, followed by all categories
from child B in a particular order, you must override
\link unity::scopes::utility::BufferedResultForwarder::push(unity::scopes::CategorisedResult) push()\endlink
to, for example, collapse categories received from a child into a single category, or otherwise buffer
results yourself until you have established the order you need.

Note that buffering fundamentally conflicts with the need to render results as soon as possible. You should
avoid buffering more data than absolutely necessary in order for the display to start updating as soon
as possible after a query was sent.

If you create a custom implementation of a forwarder, you _must_ push results via the proxy returned by
BufferedResultForwarder::upstream(). (This is a different proxy than the one that is passed to the
constructor.) Your forwarder then must declare itself ready by calling BufferedResultForwarder::set_ready()
as soon as it knows it will not push results for any new categories. In the case where your aggregator
aggregates all results from given child scope into a single category, you can call `set_ready()`
as soon as you have pushed the first result.

Here is a code example that shows how to write a result forwarder that creates a separate category for
results from each of three children and controls the order in which these categories are rendered.

\code
class SearchReceiver : public BufferedResultForwarder
{
public:
    SearchReceiver(unity::scopes::Category::SCPtr target_cat, unity::scopes::SearchReplyProxy const& upstream, BufferedResultForwarder::SPtr const&
    next_forwarder = BufferedResultForwarder::SPtr())
        : BufferedResultForwarder(upstream, next_forwarder),
          category_(target_cat)
    {
    }

    void push(CategorisedResult result) override
    {
        result.set_category(category_); // put all incoming results in single category
        upstream()->push(result); // push modified result to the client
        // we push into a single target category, so we're ready as soon as we received and pushed first result
        set_ready();
    }

private:
    unity::scopes::Category::SCPtr category_;
};

void AggregatorSearchQueryBase::run(SearchReplyProxy const& upstream_reply)
{
    auto cat1 = upstream_reply->register_category("cat1", "Results from scope 1", "", CategoryRenderer());
    auto cat2 = upstream_reply->register_category("cat2", "Results from scope 2", "", CategoryRenderer());
    auto cat3 = upstream_reply->register_category("cat3", "Results from scope 3", "", CategoryRenderer());

    // note: the order of construction is reversed
    auto scope3fwd = std::make_shared<SearchReceiver>(cat3, upstream_reply);
    auto scope2fwd = std::make_shared<SearchReceiver>(cat2, upstream_reply, scope3fwd);
    auto scope1fwd = std::make_shared<SearchReceiver>(cat1, upstream_reply, scope2fwd);

    // invoke search for child scopes; make sure you do this only after all forwarders are created
    subsearch(scope1proxy, "", scope1fwd);
    subsearch(scope2proxy, "", scope2fwd);
    subsearch(scope3proxy, "", scope3fwd);
}

\endcode

\see SearchListenerBase.
*/

BufferedResultForwarder::BufferedResultForwarder(unity::scopes::SearchReplyProxy const& upstream, BufferedResultForwarder::SPtr const& next_forwarder)
    : SearchListenerBase(),
      p(new internal::BufferedResultForwarderImpl(upstream, next_forwarder))
{
}

BufferedResultForwarder::~BufferedResultForwarder() = default;

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

unity::scopes::SearchReplyProxy BufferedResultForwarder::upstream() const
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

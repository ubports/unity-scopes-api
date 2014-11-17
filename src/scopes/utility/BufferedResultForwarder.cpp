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

/*!
\class BufferedResultForwarder

\brief Base class for a client to receive and buffer the results of a query until another BufferedResultForwarder becomes ready.

This class implements result buffering, useful for aggregator scopes which receive results from multiple child scopes and
want them displayed in specific order (in terms of Category order).

Categories (unity::scopes::Category) are displayed in the order their results are pushed - this usually poses a challenge for aggreagor scopes,
because results from child scopes arrive asynchronously in random order and therefore need to be buffered and re-ordered before pushing
to appear in desired order. This can easily be achieved with BufferedResultForwarder: aggregator scope needs to create an instance of BufferedResultForwarder
for every child scope it queries. The instances of BufferedResultForwarder need to be chained by passing next
forwarder to the constructor of prior forwarder. Chaining constitutes the order in which results wil be pushed to the
upstream SearchReply object.

The default implementation of BufferedResultForwarder just forwards the results it recevies upstream and immediately declares itself "ready" after receiving the
first result; the results are then buffered till all prior forwarders declare themselves ready. Buffering is very efficient - results are only buffered
for as long as required to ensure proper order and buffering is disabled for forwarders which don't need to wait for other forwarders to be ready. That means
that results will be pushed to the client (displayed) as soon as possible.

Please note, that the default implementation only pushes results (and categories) and ignores departments, filters and annotations. If you wish to handle and
forward those to the client, you need to derive from BufferedResultForwarder and override respective methods of base unity::scopes::SearchListenerBase
interface. Also note, that since the default implementation just forwards received results, it will recreate all the received categories from all child scopes,
and the order defined by BufferedResultForwarder chaining will only be guaranteed among first received category of each child scope. Therefore in
many cases it may be neccessary to derive from BufferedResultForwarder and reimplement
unity::scopes::utility::BufferedResultForwarder::push(unity::scopes::CategorisedResult) to collapse categories received from a child into one target category of
the aggregating scope.

The implementation of unity::scopes::utility::BufferedResultForwarder::push(unity::scopes::CategorisedResult) should push results to
the unity::scopes::SearchReplyProxy reply proxy returned by BufferedResultForwarder::upstream() (note that this is a different proxy
instance that the one passed to BufferedResultForwarder constructor and it is the one actually responsible for buffering). It should then declare itself ready by
calling BufferedResultForwarder::set_ready() as soon as it knows it won't push results for any new categories. In typical cases, where
aggregator scopes aggregates all results from given child scope into a single category, it can call set_ready() immediately after pushing first reasult in that
category upstream.

A sample code for aggregating results from three scopes (no matter how many categories they create) into three aggregated categories ("Results from scope1",
"Results from scope2" and "Results from scope3" - in that fixed order) may look like this:

\code
class SearchReceiver : public BufferedResultForwarder
{
public:
    SearchReceiver(unity::scopes::Category::SCPtr target_cat, unity::scopes::SearchReplyProxy const& upstream)
        : BufferedResultForwarder(upstream),
          category_(target_cat)
    {
    }

    SearchReceiver(unity::scopes::Category::SCPtr target_cat, unity::scopes::SearchReplyProxy const& upstream, BufferedResultForwarder::SPtr const& next_forwarder)
        : BufferedResultForwarder(upstream, next_forwarder),
          category_(target_cat)
    {
    }

    void push(CategorisedResult result) override
    {
        result.set_category(category_); // put all incoming results in single category
        upstream()->push(result);
        // we push into a single target category, so we're ready as soon as we push first result
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

    // invoke search for child scopes
    subsearch(scope1proxy, "", scope1fwd);
    subsearch(scope2proxy, "", scope2fwd);
    subsearch(scope3proxy, "", scope3fwd);
}

\endcode

\see SearchListenerBase.
*/

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


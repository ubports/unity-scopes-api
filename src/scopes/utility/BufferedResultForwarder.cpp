#include <unity/scopes/utility/BufferedResultForwarder.h>
#include <unity/scopes/CategorisedResult.h>
#include <unity/scopes/FilterState.h>
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

BufferedResultForwarder::BufferedResultForwarder(internal::BufferedResultForwarderImpl *impl)
    : p(impl)
{
}


void BufferedResultForwarder::attach_after(BufferedResultForwarder::SPtr const& previous_forwarder)
{
    p->attach_after(previous_forwarder);
}

bool BufferedResultForwarder::is_ready() const
{
    return p->is_ready();
}

void BufferedResultForwarder::set_ready()
{
    p->set_ready();
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


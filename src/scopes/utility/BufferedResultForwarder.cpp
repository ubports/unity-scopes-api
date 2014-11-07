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

BufferedResultForwarder::BufferedResultForwarder(BufferedResultForwarder::SPtr const& previous_forwarder, unity::scopes::SearchReplyProxy const& upstream)
    : p(new internal::BufferedResultForwarderImpl(previous_forwarder, upstream))
{
}

BufferedResultForwarder::BufferedResultForwarder(internal::BufferedResultForwarderImpl *impl)
    : p(impl)
{
}

bool BufferedResultForwarder::is_ready() const
{
    return p->is_ready();
}

void BufferedResultForwarder::set_ready()
{
    p->set_ready();
    // notifies attached forwarder
    // flushes buffers
    // disables buffering
}

void BufferedResultForwarder::push_upstream(Department::SCPtr const& parent)
{
    p->push_upstream(parent);
}

void BufferedResultForwarder::push_upstream(CategorisedResult result)
{
    p->push_upstream(result);
}

void BufferedResultForwarder::push_upstream(experimental::Annotation annotation)
{
    p->push_upstream(annotation);
}

void BufferedResultForwarder::push_upstream(Category::SCPtr const& category)
{
    p->push_upstream(category);
}

void BufferedResultForwarder::push_upstream(Filters const& filters, FilterState const& filter_state)
{
    p->push_upstream(filters, filter_state);
}

void BufferedResultForwarder::finished(CompletionDetails const& details)
{
    p->finished(details);
}

/*
void BufferedResultForwarder::push(Department::SCPtr const& parent)
{
}

void BufferedResultForwarder::push(CategorisedResult result)
{
}

void BufferedResultForwarder::push(experimental::Annotation annotation)
{
}

void BufferedResultForwarder::push(Category::SCPtr const& category)
{
}

void BufferedResultForwarder::push(Filters const& filters, FilterState const& filter_state)
{
}*/

}

}

}


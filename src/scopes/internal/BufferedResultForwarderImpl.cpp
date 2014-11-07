#include <unity/scopes/internal/BufferedResultForwarderImpl.h>
#include <unity/scopes/internal/BufferedSearchReplyImpl.h>
#include <unity/scopes/SearchReply.h>

namespace unity
{

namespace scopes
{

namespace internal
{

BufferedResultForwarderImpl::BufferedResultForwarderImpl(utility::BufferedResultForwarder::SPtr const& previous_forwarder, unity::scopes::SearchReplyProxy const& upstream)
    : ready_(false),
      buffer_(true),
      upstream_(std::make_shared<internal::BufferedSearchReplyImpl>(upstream))
{
}

BufferedResultForwarderImpl::BufferedResultForwarderImpl(unity::scopes::SearchReplyProxy const& upstream)
    : ready_(false),
      buffer_(true),
      upstream_(std::make_shared<internal::BufferedSearchReplyImpl>(upstream))
{
}

unity::scopes::SearchReplyProxy const& BufferedResultForwarderImpl::upstream()
{
    return upstream_;
}

bool BufferedResultForwarderImpl::is_ready() const
{
    utility::BufferedResultForwarder::SPtr prev(prev_.lock());
    if (prev)
    {
        return ready_ && prev->is_ready();
    }
    return ready_;
}

void BufferedResultForwarderImpl::set_ready()
{
    if (!ready_)
    {
        ready_ = true;

        // notify next forwarder that this one is ready
        utility::BufferedResultForwarder::SPtr next(next_.lock());
        if (next)
        {
            next->p->notify_ready();
        }
    }
}

void BufferedResultForwarderImpl::notify_ready()
{
    // we got notified that previous forwarder is ready;
    // if all preceding forwarders and this one are ready, then
    // disable buffering and flush.
    if (is_ready())
    {
        buffer_ = false; // disable buffering from now on
        flush();
    }
}

void BufferedResultForwarderImpl::flush()
{
    for (auto const& res: results_)
    {
        upstream_->push(res);
    }
    results_.clear();
}

void BufferedResultForwarderImpl::push_upstream(Department::SCPtr const& parent)
{
}

void BufferedResultForwarderImpl::push_upstream(CategorisedResult result)
{
    if (buffer_)
    {
        results_.push_back(result);
    }
    else
    {
        upstream_->push(result);
    }
}

void BufferedResultForwarderImpl::push_upstream(experimental::Annotation annotation)
{
}

void BufferedResultForwarderImpl::push_upstream(Category::SCPtr const& category)
{
}

void BufferedResultForwarderImpl::push_upstream(Filters const& filters, FilterState const& filter_state)
{
}

void BufferedResultForwarderImpl::finished(CompletionDetails const&)
{
    set_ready();
}

}

}

}

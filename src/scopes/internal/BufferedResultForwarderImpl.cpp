#include <unity/scopes/internal/BufferedResultForwarderImpl.h>
#include <unity/scopes/internal/BufferedSearchReplyImpl.h>
#include <unity/scopes/SearchReply.h>
#include <cassert>

namespace unity
{

namespace scopes
{

namespace internal
{

BufferedResultForwarderImpl::BufferedResultForwarderImpl(unity::scopes::SearchReplyProxy const& upstream)
    : ready_(false),
      upstream_(std::make_shared<internal::BufferedSearchReplyImpl>(upstream))
{
}

void BufferedResultForwarderImpl::attach_after(BufferedResultForwarder::SPtr const& previous_forwarder)
{
    prev_ = previous_forwarder;
    //prev_->next_ = 
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
        // note: this recursively visits all forwarders on the list via prev pointers
        return ready_ && prev->is_ready();
    }
    return ready_;
}

void BufferedResultForwarderImpl::set_ready()
{
    if (!ready_)
    {
        ready_ = true;

        if (is_ready()) // if preceding forwarders are all ready then flush
        {
            flush_and_notify();
        }
    }
}

void BufferedResultForwarderImpl::notify_ready()
{
    // we got notified that previous forwarder is ready;
    // if we are ready then notify following forwarders
    if (ready_)
    {
        flush_and_notify();
    }
}

void BufferedResultForwarderImpl::flush_and_notify()
{
    auto buf = std::dynamic_pointer_cast<internal::BufferedSearchReplyImpl>(upstream_);
    assert(buf);

    buf->disable_buffer();
    buf->flush();

    // notify next forwarder that this one is ready
    utility::BufferedResultForwarder::SPtr next(next_.lock());
    if (next)
    {
        next->p->notify_ready();
    }
}

void BufferedResultForwarderImpl::finished(CompletionDetails const&)
{
    set_ready();
}

}

}

}

#pragma once

#include <unity/scopes/Department.h>
#include <unity/scopes/Annotation.h>
#include <unity/scopes/Category.h>
#include <unity/scopes/CategorisedResult.h>
#include <unity/scopes/FilterBase.h>
#include <unity/scopes/SearchReplyProxyFwd.h>
#include <unity/scopes/utility/BufferedResultForwarder.h>
#include <unity/scopes/ReplyProxyFwd.h>

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

    void attach_after(BufferedResultForwarder::SPtr const& previous_forwarder);
    unity::scopes::SearchReplyProxy const& upstream();
    bool is_ready() const;
    void set_ready();
    void notify_ready();
    void flush_and_notify();
    void finished(CompletionDetails const& details);

private:
    bool ready_;
    unity::scopes::SearchReplyProxy const upstream_;
    std::weak_ptr<utility::BufferedResultForwarder> prev_;
    std::weak_ptr<utility::BufferedResultForwarder> next_;
};

}

}

}

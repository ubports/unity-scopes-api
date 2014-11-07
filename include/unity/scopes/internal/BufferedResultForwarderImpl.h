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
    BufferedResultForwarderImpl(utility::BufferedResultForwarder::SPtr const& previous_forwarder, unity::scopes::SearchReplyProxy const& upstream);
    BufferedResultForwarderImpl(unity::scopes::SearchReplyProxy const& upstream);

    unity::scopes::SearchReplyProxy const& upstream();
    bool is_ready() const;
    void set_ready();

    void push_upstream(Department::SCPtr const& parent);
    void push_upstream(CategorisedResult result);
    void push_upstream(experimental::Annotation annotation);
    void push_upstream(Category::SCPtr const& category);
    void push_upstream(Filters const& filters, FilterState const& filter_state);


    void finished(CompletionDetails const& details);

    void notify_ready();
    void flush();

private:
    bool ready_;
    bool buffer_;
    unity::scopes::SearchReplyProxy const upstream_;
    std::vector<CategorisedResult> results_;
    std::weak_ptr<utility::BufferedResultForwarder> prev_;
    std::weak_ptr<utility::BufferedResultForwarder> next_;
};

}

}

}

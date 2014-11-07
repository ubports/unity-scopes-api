#pragma once

/*
   auto scope1fwd = make_shared<BufferedResultForwarder>(upstream_reply);
   auto scope2fwd = make_shared<BufferedResultForwarder>(upstream_reply, scope1fwd); // force order

   subsearch(scope1proxy, "", "", FilterState(), metadata, scope1fwd);
   subsearch(scope2proxy, "", "", FilterState(), metadata, scope2fwd);
 */

#include <unity/scopes/SearchListenerBase.h>
#include <unity/scopes/SearchReplyProxyFwd.h>
#include <unity/scopes/Category.h>
#include <unity/scopes/Department.h>

namespace unity
{

namespace scopes
{

namespace experimental
{
    class Annotation;
} // namespace experimental

namespace internal
{
    class BufferedResultForwarderImpl;
}

namespace utility
{

class BufferedResultForwarder : public unity::scopes::SearchListenerBase
{
public:
    /// @cond
    NONCOPYABLE(BufferedResultForwarder);
    UNITY_DEFINES_PTRS(BufferedResultForwarder);
    /// @endcond

    BufferedResultForwarder(unity::scopes::SearchReplyProxy const& upstream);
    BufferedResultForwarder(BufferedResultForwarder::SPtr const& previous_forwarder, unity::scopes::SearchReplyProxy const& upstream);

    bool is_ready() const;

protected:
    void set_ready();
    unity::scopes::SearchReplyProxy const& upstream();

    void push_upstream(Department::SCPtr const& parent);
    void push_upstream(CategorisedResult result);
    void push_upstream(experimental::Annotation annotation);
    void push_upstream(Category::SCPtr const& category);
    void push_upstream(Filters const& filters, FilterState const& filter_state);

    void finished(CompletionDetails const& details) override;

/*private:
    void push(Department::SCPtr const& parent) override;
    void push(CategorisedResult result) override;
    void push(experimental::Annotation annotation) override;
    void push(Category::SCPtr const& category) override;
    void push(Filters const& filters, FilterState const& filter_state) override;*/

private:
    BufferedResultForwarder(internal::BufferedResultForwarderImpl* impl);
    friend class internal::BufferedResultForwarderImpl;

    internal::BufferedResultForwarderImpl *p;
};

} // namespace utility

} // namespace scopes

} // namespace unity

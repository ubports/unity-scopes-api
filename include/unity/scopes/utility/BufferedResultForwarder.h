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

    void attach_after(BufferedResultForwarder::SPtr const& previous_forwarder);
    bool is_ready() const;

protected:
    void set_ready();
    unity::scopes::SearchReplyProxy const& upstream();

    void finished(CompletionDetails const& details) override;

private:
    BufferedResultForwarder(internal::BufferedResultForwarderImpl* impl);
    friend class internal::BufferedResultForwarderImpl;

    internal::BufferedResultForwarderImpl *p;
};

} // namespace utility

} // namespace scopes

} // namespace unity

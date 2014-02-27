/*
 * Copyright (C) 2013 Canonical Ltd
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
 * Authored by: Michi Henning <michi.henning@canonical.com>
 */

#ifndef UNITY_SCOPES_INTERNAL_REPLYIMPL_H
#define UNITY_SCOPES_INTERNAL_REPLYIMPL_H

#include <unity/scopes/internal/MWReplyProxyFwd.h>
#include <unity/scopes/internal/CategoryRegistry.h>
#include <unity/scopes/internal/ObjectProxyImpl.h>
#include <unity/scopes/ReplyProxyFwd.h>
#include <unity/scopes/Category.h>
#include <unity/scopes/PreviewWidget.h>
#include <unity/scopes/ColumnLayout.h>
#include <unity/scopes/ListenerBase.h>
#include <unity/scopes/Department.h>
#include <unity/scopes/FilterBase.h>
#include <unity/scopes/FilterState.h>

#include <atomic>

namespace unity
{

namespace scopes
{

class CategorisedResult;
class CategoryRenderer;
class Annotation;

namespace internal
{

class QueryObjectBase;

// Proxy used by a scope to push results for a query.
// To indicate that it has sent the last result, the scope must call finished().
// Subsequent calls to finished() are ignored.
// Calls to push() after finished() was called are ignored.
// If the proxy goes out of scope before finished was called, it implicitly calls finished().

class ReplyImpl : public virtual ObjectProxyImpl
{
public:
    ReplyImpl(MWReplyProxy const& mw_proxy, std::shared_ptr<QueryObjectBase>const & qo);
    virtual ~ReplyImpl();

    void register_departments(DepartmentList const& departments, std::string current_department_id);
    Category::SCPtr register_category(std::string const& id, std::string const& title, std::string const &icon, CategoryRenderer const& renderer_template);
    void register_category(Category::SCPtr category);
    Category::SCPtr lookup_category(std::string const& id) const;

    bool push(unity::scopes::CategorisedResult const& result);
    bool register_annotation(unity::scopes::Annotation const& annotation);
    bool register_layout(unity::scopes::ColumnLayoutList const& layouts);
    bool push(unity::scopes::PreviewWidgetList const& widgets);
    bool push(std::string const& key, Variant const& value);
    bool push(unity::scopes::Filters const& filters, unity::scopes::FilterState const& filter_state);

    void finished();
    void finished(unity::scopes::ListenerBase::Reason reason);
    void error(std::exception_ptr ex);

    static SearchReplyProxy create(MWReplyProxy const& mw_proxy, std::shared_ptr<QueryObjectBase> const& qo);
    static PreviewReplyProxy create_preview_reply(MWReplyProxy const& mw_proxy, std::shared_ptr<QueryObjectBase> const& qo);

    typedef std::function<void()> CleanupFunc;

private:
    bool push(Category::SCPtr category);
    bool push(VariantMap const& variant_map);
    MWReplyProxy fwd() const;

    std::shared_ptr<QueryObjectBase> qo_;
    std::shared_ptr<CategoryRegistry> cat_registry_;
    std::atomic_bool finished_;
    bool layouts_push_disallowed_;

    std::atomic_int cardinality_;
    std::atomic_int num_pushes_;
};

} // namespace internal

} // namespace scopes

} // namespace unity

#endif

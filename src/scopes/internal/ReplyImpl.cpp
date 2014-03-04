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

#include <unity/scopes/Annotation.h>
#include <unity/scopes/CategorisedResult.h>
#include <unity/scopes/CategoryRenderer.h>
#include <unity/scopes/CategoryRenderer.h>
#include <unity/scopes/internal/ColumnLayoutImpl.h>
#include <unity/scopes/internal/DepartmentImpl.h>
#include <unity/scopes/internal/FilterStateImpl.h>
#include <unity/scopes/internal/MiddlewareBase.h>
#include <unity/scopes/internal/MWReply.h>
#include <unity/scopes/internal/PreviewReply.h>
#include <unity/scopes/internal/QueryObjectBase.h>
#include <unity/scopes/internal/ReplyImpl.h>
#include <unity/scopes/internal/RuntimeImpl.h>
#include <unity/scopes/ReplyBase.h>
#include <unity/scopes/ReplyProxyFwd.h>
#include <unity/scopes/SearchReply.h>
#include <unity/UnityExceptions.h>

#include <cassert>
#include <sstream>
#include <iostream> // TODO: remove this once logging is added

using namespace std;

namespace unity
{

namespace scopes
{

namespace internal
{

ReplyImpl::ReplyImpl(MWReplyProxy const& mw_proxy, std::shared_ptr<QueryObjectBase> const& qo) :
    ObjectProxyImpl(mw_proxy),
    qo_(qo),
    cat_registry_(new CategoryRegistry()),
    finished_(false),
    layouts_push_disallowed_(false),
    cardinality_(qo->cardinality({ fwd()->identity(), fwd()->mw_base() })),
    num_pushes_(0)
{
    assert(mw_proxy);
    assert(cardinality_ >= 0);
}

ReplyImpl::~ReplyImpl()
{
    try
    {
        finished();
    }
    catch (...)
    {
        // TODO: log error
    }
}

void ReplyImpl::register_category(Category::SCPtr category)
{
    cat_registry_->register_category(category); // will throw if that category id has already been registered
    push(category);
}

void ReplyImpl::register_departments(DepartmentList const& departments, std::string current_department_id)
{
    // basic consistency check
    try
    {
        DepartmentImpl::validate_departments(departments, current_department_id);
    }
    catch (unity::LogicException const &e)
    {
        throw unity::LogicException("Reply::register_departments(): Failed to validate departments");
    }

    push(internal::DepartmentImpl::serialize_departments(departments, current_department_id)); // ignore return value?
}

Category::SCPtr ReplyImpl::register_category(std::string const& id,
                                             std::string const& title,
                                             std::string const &icon,
                                             CategoryRenderer const& renderer_template)
{
    auto cat = cat_registry_->register_category(id, title, icon, renderer_template); // will throw if adding same category again
    push(cat);
    return cat;
}

Category::SCPtr ReplyImpl::lookup_category(std::string const& id) const
{
    return cat_registry_->lookup_category(id);
}

bool ReplyImpl::push(unity::scopes::CategorisedResult const& result)
{
    // If this is an aggregator scope, it may be pushing result items obtained
    // from ReplyObject without registering a category first.
    if (cat_registry_->lookup_category(result.category()->id()) == nullptr)
    {
        register_category(result.category());
    }

    VariantMap var;
    var["result"] = result.serialize();
    if (!push(var))
    {
        return false;
    }

    // Enforce cardinality limit (0 means no limit). If the scope pushes more results
    // than requested, future pushes are ignored. push() returns false
    // on the last call that actually still pushed a result.
    // To the client, a query that exceeds the limit looks like a query
    // that returned the maximum number of results and finished normally.
    if (++num_pushes_ == cardinality_)
    {
        // At most one thread will execute this.
        finished();
        return false;  // This was the last successful push
    }
    return true;
}

bool ReplyImpl::push(Category::SCPtr category)
{
    VariantMap var;
    var["category"] = category->serialize();
    return push(var);
}

bool ReplyImpl::register_annotation(unity::scopes::Annotation const& annotation)
{
    VariantMap var;
    var["annotation"] = annotation.serialize();
    return push(var);
}

bool ReplyImpl::push(unity::scopes::Filters const& filters, unity::scopes::FilterState const& filter_state)
{
    VariantMap var;
    VariantArray filters_var;

    for (auto const& f: filters)
    {
        filters_var.push_back(Variant(f->serialize()));
    }
    var["filters"] = filters_var;
    var["filter_state"] = filter_state.serialize();
    return push(var);
}

bool ReplyImpl::register_layout(unity::scopes::ColumnLayoutList const& layouts)
{
    if (layouts_push_disallowed_)
    {
        throw unity::LogicException("Reply::register_layout(): column layouts can only be registered once and before pushing preview widgets");
    }

    // basic check for consistency of layouts
    try
    {
        ColumnLayoutImpl::validate_layouts(layouts);
    }
    catch (unity::LogicException const &e)
    {
        throw unity::LogicException("Reply::register_layout(): Failed to validate layouts");
    }

    VariantMap vm;
    VariantArray arr;
    for (auto const& layout: layouts)
    {
        arr.push_back(Variant(layout.serialize()));
    }
    vm["columns"] = arr;
    return layouts_push_disallowed_ = push(vm);
}

bool ReplyImpl::push(unity::scopes::PreviewWidgetList const& widgets)
{
    layouts_push_disallowed_ = true;

    VariantMap vm;
    VariantArray arr;
    for (auto const& widget : widgets)
    {
        arr.push_back(Variant(widget.serialize()));
    }
    vm["widgets"] = arr;
    return push(vm);
}

bool ReplyImpl::push(std::string const& key, Variant const& value)
{
    VariantMap vm;
    VariantMap nested;
    nested[key] = value;
    vm["preview-data"] = nested;
    return push(vm);
}

bool ReplyImpl::push(VariantMap const& variant_map)
{
    auto qo = dynamic_pointer_cast<QueryObjectBase>(qo_);
    assert(qo);
    if (!qo->pushable({ fwd()->identity(), fwd()->mw_base() }))
    {
        return false; // Query was cancelled or had an error.
    }

    if (finished_)
    {
        return false;
    }

    try
    {
        fwd()->push(variant_map);
    }
    catch (std::exception const&)
    {
        error(current_exception());
        return false;
    }

    return true;
}

void ReplyImpl::finished()
{
    finished(ListenerBase::Finished);
}

void ReplyImpl::finished(ListenerBase::Reason reason)
{
    if (!finished_.exchange(true))
    {
        try
        {
            fwd()->finished(reason, "");
        }
        catch (std::exception const& e)
        {
            // TODO: log error
            cerr << e.what() << endl;
        }
    }
}

void ReplyImpl::error(exception_ptr ex)
{
    if (finished_.exchange(true))
    {
        // Only the first thread to encounter an error
        // reports the error to the client.
        return;
    }

    string error_message;
    try
    {
        rethrow_exception(ex);
    }
    catch (std::exception const& e)
    {
        error_message = e.what();
    }
    catch (...)
    {
        error_message = "unknown exception";
    }
    // TODO: log error
    cerr << error_message << endl;

    try
    {
        fwd()->finished(ListenerBase::Error, error_message);
    }
    catch (std::exception const& e)
    {
        // TODO: log error
        cerr << e.what() << endl;
    }
}

SearchReplyProxy ReplyImpl::create(MWReplyProxy const& mw_proxy, std::shared_ptr<QueryObjectBase> const& qo)
{
    auto reply_impl = new ReplyImpl(mw_proxy, qo);
    auto reply = new SearchReply(reply_impl);
    return SearchReplyProxy(reply);
}

PreviewReplyProxy ReplyImpl::create_preview_reply(MWReplyProxy const& mw_proxy, std::shared_ptr<QueryObjectBase> const& qo)
{
    auto reply_impl = new ReplyImpl(mw_proxy, qo);
    auto reply = new PreviewReply(reply_impl);
    return PreviewReplyProxy(reply);
}

MWReplyProxy ReplyImpl::fwd() const
{
    return dynamic_pointer_cast<MWReply>(proxy());
}

} // namespace internal

} // namespace scopes

} // namespace unity

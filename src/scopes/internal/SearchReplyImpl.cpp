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

#include <unity/scopes/internal/SearchReplyImpl.h>

#if 0
#include <unity/scopes/Annotation.h>
#include <unity/scopes/CategorisedResult.h>
#include <unity/scopes/CategoryRenderer.h>
#include <unity/scopes/CategoryRenderer.h>
#include <unity/scopes/internal/ColumnLayoutImpl.h>
#endif
#include <unity/scopes/internal/DepartmentImpl.h>
#if 0
#include <unity/scopes/internal/FilterStateImpl.h>
#include <unity/scopes/internal/MiddlewareBase.h>
#endif
#include <unity/scopes/internal/MWReply.h>
#if 0
#include <unity/scopes/internal/PreviewReplyImpl.h>
#include <unity/scopes/internal/QueryObjectBase.h>
#include <unity/scopes/internal/RuntimeImpl.h>
#include <unity/scopes/internal/SearchReplyImpl.h>
#include <unity/scopes/PreviewReply.h>
#include <unity/scopes/Reply.h>
#include <unity/scopes/ReplyProxyFwd.h>
#include <unity/scopes/SearchReply.h>
#include <unity/UnityExceptions.h>

#include <cassert>
#include <sstream>
#include <iostream> // TODO: remove this once logging is added
#endif

using namespace std;

namespace unity
{

namespace scopes
{

namespace internal
{

SearchReplyImpl::SearchReplyImpl(MWReplyProxy const& mw_proxy, std::shared_ptr<QueryObjectBase> const& qo) :
    ObjectImpl(mw_proxy),
    ReplyImpl(mw_proxy, qo)
{
}

SearchReplyImpl::~SearchReplyImpl()
{
}

void SearchReplyImpl::register_category(Category::SCPtr category)
{
#if 0
    cat_registry_->register_category(category); // will throw if that category id has already been registered
    push(category);
#else
    ReplyImpl::register_category(category);
#endif
}

void SearchReplyImpl::register_departments(DepartmentList const& departments, std::string current_department_id)
{
#if 0
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
#else
    ReplyImpl::register_departments(departments, current_department_id);
#endif
}

Category::SCPtr SearchReplyImpl::register_category(std::string const& id,
                                             std::string const& title,
                                             std::string const &icon,
                                             CategoryRenderer const& renderer_template)
{
#if 0
    auto cat = cat_registry_->register_category(id, title, icon, renderer_template); // will throw if adding same category again
    push(cat);
    return cat;
#else
    return ReplyImpl::register_category(id, title, icon, renderer_template);
#endif
}

Category::SCPtr SearchReplyImpl::lookup_category(std::string const& id)
{
#if 0
    return cat_registry_->lookup_category(id);
#else
    return ReplyImpl::lookup_category(id);
#endif
}

bool SearchReplyImpl::push(unity::scopes::CategorisedResult const& result)
{
#if 0
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
#else
    return ReplyImpl::push(result);
#endif
}

#if 0
bool SearchReplyImpl::push(Category::SCPtr category)
{
    VariantMap var;
    var["category"] = category->serialize();
    return push(var);
}
#endif

bool SearchReplyImpl::register_annotation(unity::scopes::Annotation const& annotation)
{
#if 0
    VariantMap var;
    var["annotation"] = annotation.serialize();
    return push(var);
#else
    return ReplyImpl::register_annotation(annotation);
#endif
}

bool SearchReplyImpl::push(unity::scopes::Filters const& filters, unity::scopes::FilterState const& filter_state)
{
#if 0
    VariantMap var;
    VariantArray filters_var;

    for (auto const& f: filters)
    {
        filters_var.push_back(Variant(f->serialize()));
    }
    var["filters"] = filters_var;
    var["filter_state"] = filter_state.serialize();
    return push(var);
#else
    return ReplyImpl::push(filters, filter_state);
#endif
}

SearchReplyProxy SearchReplyImpl::create_search_reply(MWReplyProxy const& mw_proxy, std::shared_ptr<QueryObjectBase> const& qo)
{
    return make_shared<SearchReplyImpl>(mw_proxy, qo);
}

} // namespace internal

} // namespace scopes

} // namespace unity

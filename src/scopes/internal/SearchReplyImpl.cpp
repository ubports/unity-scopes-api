/*
 * Copyright (C) 2014 Canonical Ltd
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
 * Authored by: Pawel Stolowski <pawel.stolowski@canonical.com>
 *              Michi Henning <michi.henning@canonical.com>
 */

#include <unity/scopes/internal/SearchReplyImpl.h>

#include <unity/scopes/Annotation.h>
#include <unity/scopes/CategorisedResult.h>
#include <unity/scopes/internal/DepartmentImpl.h>
#include <unity/scopes/internal/MWReply.h>
#include <unity/scopes/internal/FilterBaseImpl.h>
#include <unity/scopes/internal/QueryObjectBase.h>
#include <unity/UnityExceptions.h>

using namespace std;

namespace unity
{

namespace scopes
{

namespace internal
{

SearchReplyImpl::SearchReplyImpl(MWReplyProxy const& mw_proxy,
                                 std::shared_ptr<QueryObjectBase> const& qo,
                                 int cardinality) :
    ObjectImpl(mw_proxy),
    ReplyImpl(mw_proxy, qo),
    cat_registry_(new CategoryRegistry()),
    cardinality_(cardinality),
    num_pushes_(0)
{
}

SearchReplyImpl::~SearchReplyImpl()
{
}

void SearchReplyImpl::register_departments(DepartmentList const& departments, std::string current_department_id)
{
    // basic consistency check
    try
    {
        DepartmentImpl::validate_departments(departments, current_department_id);
    }
    catch (unity::LogicException const &e)
    {
        throw unity::LogicException("SearchReplyImpl::register_departments(): Failed to validate departments");
    }

    ReplyImpl::push(internal::DepartmentImpl::serialize_departments(departments, current_department_id)); // ignore return value?
}

void SearchReplyImpl::register_category(Category::SCPtr category)
{
    cat_registry_->register_category(category); // will throw if that category id has already been registered
    push(category);
}

Category::SCPtr SearchReplyImpl::register_category(std::string const& id,
                                             std::string const& title,
                                             std::string const &icon,
                                             CategoryRenderer const& renderer_template)
{
    // will throw if adding same category again
    auto cat = cat_registry_->register_category(id, title, icon, renderer_template);
    push(cat);
    return cat;
}

Category::SCPtr SearchReplyImpl::lookup_category(std::string const& id)
{
    return cat_registry_->lookup_category(id);
}

bool SearchReplyImpl::push(unity::scopes::Annotation const& annotation)
{
    VariantMap var;
    var["annotation"] = annotation.serialize();
    return ReplyImpl::push(var);
}

bool SearchReplyImpl::push(unity::scopes::CategorisedResult const& result)
{
    // If this is an aggregator scope, it may be pushing result items obtained
    // from ReplyObject without registering a category first.
    if (cat_registry_->lookup_category(result.category()->id()) == nullptr)
    {
        register_category(result.category());
    }

    VariantMap var;
    var["result"] = result.serialize();
    if (!ReplyImpl::push(var))
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

bool SearchReplyImpl::push(unity::scopes::Filters const& filters, unity::scopes::FilterState const& filter_state)
{
    // basic consistency check
    try
    {
        internal::FilterBaseImpl::validate_filters(filters);
    }
    catch (unity::LogicException const &e)
    {
        throw unity::LogicException("SearchReplyImpl::push(): Failed to validate filters");
    }

    VariantMap var;
    var["filters"] = internal::FilterBaseImpl::serialize_filters(filters);
    var["filter_state"] = filter_state.serialize();
    return ReplyImpl::push(var);
}

bool SearchReplyImpl::push(Category::SCPtr category)
{
    VariantMap var;
    var["category"] = category->serialize();
    return ReplyImpl::push(var);
}

} // namespace internal

} // namespace scopes

} // namespace unity

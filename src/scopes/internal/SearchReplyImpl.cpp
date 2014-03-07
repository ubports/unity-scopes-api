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

#include <unity/scopes/internal/DepartmentImpl.h>
#include <unity/scopes/internal/MWReply.h>

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
    ReplyImpl::register_category(category);
}

void SearchReplyImpl::register_departments(DepartmentList const& departments, std::string current_department_id)
{
    ReplyImpl::register_departments(departments, current_department_id);
}

Category::SCPtr SearchReplyImpl::register_category(std::string const& id,
                                             std::string const& title,
                                             std::string const &icon,
                                             CategoryRenderer const& renderer_template)
{
    return ReplyImpl::register_category(id, title, icon, renderer_template);
}

Category::SCPtr SearchReplyImpl::lookup_category(std::string const& id)
{
    return ReplyImpl::lookup_category(id);
}

bool SearchReplyImpl::push(unity::scopes::CategorisedResult const& result)
{
    return ReplyImpl::push(result);
}

bool SearchReplyImpl::register_annotation(unity::scopes::Annotation const& annotation)
{
    return ReplyImpl::register_annotation(annotation);
}

bool SearchReplyImpl::push(unity::scopes::Filters const& filters, unity::scopes::FilterState const& filter_state)
{
    return ReplyImpl::push(filters, filter_state);
}

} // namespace internal

} // namespace scopes

} // namespace unity

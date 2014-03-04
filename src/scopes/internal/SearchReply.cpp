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

#include <unity/scopes/internal/SearchReply.h>

#include <unity/scopes/Annotation.h>
#include <unity/scopes/CategorisedResult.h>
#include <unity/scopes/CategoryRenderer.h>
#include <unity/scopes/internal/ReplyImpl.h>

namespace unity
{

namespace scopes
{

namespace internal
{

//! @cond

SearchReply::SearchReply(ReplyImpl* impl) :
    unity::scopes::SearchReply(), Reply(impl)
{
}

SearchReply::~SearchReply()
{
}

void SearchReply::register_departments(DepartmentList const& departments, std::string current_department_id)
{
    return fwd()->register_departments(departments, current_department_id);
}

Category::SCPtr SearchReply::register_category(std::string const& id, std::string const& title, std::string const &icon, CategoryRenderer const& renderer_template)
{
    return fwd()->register_category(id, title, icon, renderer_template);
}

void SearchReply::register_category(Category::SCPtr category)
{
    fwd()->register_category(category);
}

Category::SCPtr SearchReply::lookup_category(std::string const& id) const
{
    return fwd()->lookup_category(id);
}

bool SearchReply::push(CategorisedResult const& result) const
{
    return fwd()->push(result);
}

bool SearchReply::register_annotation(Annotation const& annotation) const
{
    return fwd()->register_annotation(annotation);
}

bool SearchReply::push(Filters const& filters, FilterState const& filter_state) const
{
    return fwd()->push(filters, filter_state);
}

//! @endcond

} // namespace internal

} // namespace scopes

} // namespace unity

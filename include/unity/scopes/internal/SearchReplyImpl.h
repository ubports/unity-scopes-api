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
 * Authored by: Michi Henning <michi.henning@canonical.com>
 */

#ifndef UNITY_SCOPES_INTERNAL_SEARCHREPLYIMPL_H
#define UNITY_SCOPES_INTERNAL_SEARCHREPLYIMPL_H

#include <unity/scopes/Category.h>
#include <unity/scopes/Department.h>
#include <unity/scopes/FilterBase.h>
#include <unity/scopes/FilterState.h>
#include <unity/scopes/internal/CategoryRegistry.h>
#include <unity/scopes/internal/MWReplyProxyFwd.h>
#include <unity/scopes/internal/ObjectImpl.h>
#include <unity/scopes/internal/ReplyImpl.h>
#include <unity/scopes/SearchReply.h>

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

class SearchReplyImpl : public virtual unity::scopes::SearchReply, public virtual ReplyImpl
{
public:
    SearchReplyImpl(MWReplyProxy const& mw_proxy, std::shared_ptr<QueryObjectBase>const & qo);
    virtual ~SearchReplyImpl();

    virtual void register_departments(DepartmentList const& departments, std::string current_department_id) override;

    virtual Category::SCPtr register_category(std::string const& id,
                                              std::string const& title,
                                              std::string const &icon,
                                              CategoryRenderer const& renderer_template) override;
    virtual void register_category(Category::SCPtr category) override;
    virtual Category::SCPtr lookup_category(std::string const& id)  override;

    virtual bool register_annotation(unity::scopes::Annotation const& annotation) override;

    virtual bool push(unity::scopes::CategorisedResult const& result) override;
    virtual bool push(unity::scopes::Filters const& filters, unity::scopes::FilterState const& filter_state) override;

private:
    std::shared_ptr<CategoryRegistry> local_cat_reg_;
    std::shared_ptr<CategoryRegistry> remote_cat_reg_;

    std::atomic_int cardinality_;
    std::atomic_int num_pushes_;
};

} // namespace internal

} // namespace scopes

} // namespace unity

#endif

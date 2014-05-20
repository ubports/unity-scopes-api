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

#include <unity/scopes/internal/ResultReplyObject.h>
#include <unity/scopes/internal/RuntimeImpl.h>
#include <unity/scopes/internal/AnnotationImpl.h>
#include <unity/scopes/ListenerBase.h>
#include <unity/scopes/Category.h>
#include <unity/scopes/CategorisedResult.h>
#include <unity/scopes/internal/CategorisedResultImpl.h>
#include <unity/scopes/internal/DepartmentImpl.h>
#include <unity/scopes/FilterBase.h>
#include <unity/scopes/internal/FilterBaseImpl.h>
#include <unity/scopes/internal/FilterStateImpl.h>
#include <unity/UnityExceptions.h>

#include <cassert>

using namespace std;
using namespace unity::scopes::internal;

namespace unity
{

namespace scopes
{

namespace internal
{

ResultReplyObject::ResultReplyObject(SearchListenerBase::SPtr const& receiver,
                                     RuntimeImpl const* runtime,
                                     std::string const& scope_id,
                                     int cardinality) :
    ReplyObject(std::static_pointer_cast<ListenerBase>(receiver), runtime, scope_id),
    receiver_(receiver),
    cat_registry_(new CategoryRegistry()),
    runtime_(runtime),
    cardinality_(cardinality),
    num_pushes_(0)
{
    assert(receiver_);
    assert(runtime);
}

ResultReplyObject::~ResultReplyObject()
{
}

bool ResultReplyObject::process_data(VariantMap const& data)
{
    auto it = data.find("filters");
    if (it != data.end())
    {
        Filters const filters = FilterBaseImpl::deserialize_filters(it->second.get_array());
        it = data.find("filter_state");
        if (it != data.end())
        {
            auto filter_state = FilterStateImpl::deserialize(it->second.get_dict());
            receiver_->push(filters, filter_state);
        }
        else
        {
            throw InvalidArgumentException("ResultReplyObject::push(): filters present but missing filter_state data");
        }
    }

    it = data.find("category");
    if (it != data.end())
    {
        auto cat = cat_registry_->register_category(it->second.get_dict());
        receiver_->push(cat);
    }

    it = data.find("departments");
    if (it != data.end())
    {
        Department::SCPtr department = DepartmentImpl::create(it->second.get_dict());
        it = data.find("current_department");
        if (it != data.end())
        {
            auto const current_dep_id = it->second.get_string();
            Department::SCPtr current = (current_dep_id == department->id()) ? department : internal::DepartmentImpl::find_subdepartment_by_id(department, current_dep_id);
            if (current == nullptr)
            {
                throw InvalidArgumentException("ResultReplyObject::process_data(): cannot determine current department");
            }
            receiver_->push(department, current);
        }
        else
        {
            throw InvalidArgumentException("ResultReplyObject::process_data(): departments present but missing current_department");
        }
    }

    it = data.find("annotation");
    if (it != data.end())
    {
        auto result_var = it->second.get_dict();
        Annotation annotation(new internal::AnnotationImpl(result_var));
        receiver_->push(std::move(annotation));
    }

    it = data.find("result");
    if (it != data.end())
    {
        // Enforce cardinality limit.
        if (cardinality_ != 0 && ++num_pushes_ > cardinality_)
        {
            return true;
        }
        auto result_var = it->second.get_dict();
        auto impl = std::unique_ptr<internal::CategorisedResultImpl>(new internal::CategorisedResultImpl(*cat_registry_, result_var));

        impl->set_runtime(runtime_);
        // set result origin
        if (impl->origin().empty())
        {
            impl->set_origin(origin_proxy());
        }

        CategorisedResult result(impl.release());
        receiver_->push(std::move(result));
    }
    return false;
}

} // namespace internal

} // namespace scopes

} // namespace unity

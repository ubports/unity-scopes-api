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
#include <unity/scopes/FilterBase.h>
#include <unity/scopes/internal/FilterBaseImpl.h>
#include <unity/scopes/internal/FilterStateImpl.h>
#include <unity/UnityExceptions.h>

#include <cassert>
#include <iostream> // TODO: remove this once logging is added

using namespace std;
using namespace unity::scopes::internal;

namespace unity
{

namespace scopes
{

namespace internal
{

ResultReplyObject::ResultReplyObject(SearchListener::SPtr const& receiver, RuntimeImpl const* runtime, std::string const& scope_name) :
    ReplyObject(std::static_pointer_cast<ListenerBase>(receiver), runtime, scope_name),
    receiver_(receiver),
    cat_registry_(new CategoryRegistry()),
    runtime_(runtime)
{
    assert(runtime);
}

ResultReplyObject::~ResultReplyObject()
{
}

void ResultReplyObject::process_data(VariantMap const& data)
{
    auto it = data.find("filters");
    if (it != data.end())
    {
        auto filters_var = it->second.get_array();
        it = data.find("filter_state");
        if (it != data.end())
        {
            Filters filters;
            for (auto const& f: filters_var)
            {
                filters.push_back(FilterBaseImpl::deserialize(f.get_dict()));
            }
            try
            {
                auto filter_state = FilterStateImpl::deserialize(it->second.get_dict());
                receiver_->push(filters, filter_state);
            }
            catch (std::exception const& e)
            {
                // TODO: log this
                cerr << "ReplyObject::receiver_->push(): " << e.what() << endl;
                finished(ListenerBase::Error, e.what());
            }
        }
        else
        {
            // TODO: log this
            const std::string msg("ReplyObject::push(): filters present but missing filter_state data");
            cerr << msg << endl;
            finished(ListenerBase::Error, msg);
        }
    }

    it = data.find("category");
    if (it != data.end())
    {
        auto cat = cat_registry_->register_category(it->second.get_dict());
        receiver_->push(cat);
    }

    it = data.find("annotation");
    if (it != data.end())
    {
        auto result_var = it->second.get_dict();
        try
        {
            Annotation annotation(new internal::AnnotationImpl(*cat_registry_, result_var));
            receiver_->push(std::move(annotation));
        }
        catch (std::exception const& e)
        {
            // TODO: log this
            cerr << "ReplyObject::receiver_->push(): " << e.what() << endl;
            finished(ListenerBase::Error, e.what());
        }
    }

    it = data.find("result");
    if (it != data.end())
    {
        auto result_var = it->second.get_dict();
        try
        {
            auto impl = std::make_shared<internal::CategorisedResultImpl>(*cat_registry_, result_var);

            impl->set_runtime(runtime_);
            // set result origin
            if (impl->origin().empty())
            {
                impl->set_origin(origin_proxy());
            }

            CategorisedResult result(impl);
            receiver_->push(std::move(result));
        }
        catch (std::exception const& e)
        {
            // TODO: log this
            cerr << "ReplyObject::receiver_->push(): " << e.what() << endl;
            finished(ListenerBase::Error, e.what());
        }
    }
}

} // namespace internal

} // namespace scopes

} // namespace unity

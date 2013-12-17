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

#include <scopes/internal/ReplyImpl.h>

#include <scopes/internal/MiddlewareBase.h>
#include <scopes/internal/MWReply.h>
#include <scopes/internal/RuntimeImpl.h>
#include <scopes/CategorisedResult.h>
#include <scopes/Annotation.h>
#include <scopes/ScopeExceptions.h>
#include <unity/UnityExceptions.h>
#include <scopes/Reply.h>
#include <scopes/CategoryRenderer.h>

#include <sstream>
#include <cassert>

using namespace std;

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

ReplyImpl::ReplyImpl(MWReplyProxy const& mw_proxy, std::shared_ptr<QueryObject> const& qo) :
    ObjectProxyImpl(mw_proxy),
    qo_(qo),
    cat_registry_(new CategoryRegistry()),
    finished_(false)
{
    assert(mw_proxy);
    assert(qo);
}

ReplyImpl::~ReplyImpl() noexcept
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

Category::SCPtr ReplyImpl::register_category(std::string const& id, std::string const& title, std::string const &icon, CategoryRenderer const& renderer_template)
{
    auto cat = cat_registry_->register_category(id, title, icon, renderer_template); // will throw if adding same category again

    // return category instance only if pushed successfuly (i.e. search wasn't finished)
    if (push(cat))
    {
        return cat;
    }

    return nullptr;
}

Category::SCPtr ReplyImpl::lookup_category(std::string const& id) const
{
    return cat_registry_->lookup_category(id);
}

bool ReplyImpl::push(unity::api::scopes::CategorisedResult const& result)
{
    // If this is an aggregator scope, it may be pushing result items obtained
    // from ReplyObject without registering a category first.
    auto cat = cat_registry_->lookup_category(result.category()->id());
    if (cat == nullptr)
    {
        std::ostringstream s;
        s << "Unknown category " << result.category()->id();
        throw InvalidArgumentException(s.str());
    }

    VariantMap var;
    var["result"] = result.serialize();
    return push(var);
}

bool ReplyImpl::push(Category::SCPtr category)
{
    VariantMap var;
    var["category"] = category->serialize();
    return push(var);
}

bool ReplyImpl::push(unity::api::scopes::Annotation const& annotation)
{
    VariantMap var;
    var["annotation"] = annotation.serialize();
    return push(var);
}

bool ReplyImpl::push(unity::api::scopes::Filters const& filters, unity::api::scopes::FilterState const& filter_state)
{
    VariantMap var;
    VariantArray filters_var;

    for (auto const& f: filters)
    {
        filters_var.push_back(Variant(f.serialize()));
    }
    var["filters"] = filters_var;
    return push(var);
}

bool ReplyImpl::push(VariantMap const& variant_map)
{
    if (!qo_->pushable())
    {
        return false; // Query was cancelled or had an error.
    }

    if (!finished_.load())
    {
        try
        {
            fwd()->push(variant_map);
            return true;
        }
        catch (MiddlewareException const& e)
        {
            // TODO: log error
            finished(ReceiverBase::Error);
            return false;
        }
    }
    return false;
}

void ReplyImpl::finished()
{
    finished(ReceiverBase::Finished);
}

void ReplyImpl::finished(ReceiverBase::Reason reason)
{
    if (!finished_.exchange(true))
    {
        try
        {
            fwd()->finished(reason);
        }
        catch (MiddlewareException const& e)
        {
            // TODO: log error
        }
    }
}

ReplyProxy ReplyImpl::create(MWReplyProxy const& mw_proxy, std::shared_ptr<QueryObject> const& qo)
{
    return ReplyProxy(new Reply((new ReplyImpl(mw_proxy, qo))));
}

MWReplyProxy ReplyImpl::fwd() const
{
    return dynamic_pointer_cast<MWReply>(proxy());
}

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

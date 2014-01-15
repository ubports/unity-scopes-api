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

#include <scopes/Reply.h>
#include <scopes/CategorisedResult.h>
#include <scopes/Annotation.h>
#include <scopes/CategoryRenderer.h>

#include <scopes/internal/ReplyImpl.h>

#include <cassert>

namespace unity
{

namespace api
{

namespace scopes
{

//! @cond

ReplyBase::ReplyBase(internal::ReplyImpl* impl) :
    ObjectProxy(impl)
{
    assert(impl);
}

ReplyBase::~ReplyBase() noexcept
{
}

void ReplyBase::finished() const
{
    return fwd()->finished();
}

void ReplyBase::error(std::exception_ptr ex) const
{
    return fwd()->error(ex);
}

internal::ReplyImpl* ReplyBase::fwd() const
{
    return dynamic_cast<internal::ReplyImpl*>(pimpl());
}

SearchReply::SearchReply(internal::ReplyImpl* impl) :
    ObjectProxy(impl), ReplyBase(impl)
{
}

SearchReply::~SearchReply() noexcept
{
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

bool SearchReply::push(Annotation const& annotation) const
{
    return fwd()->push(annotation);
}

//! @endcond

} // namespace scopes

} // namespace api

} // namespace unity

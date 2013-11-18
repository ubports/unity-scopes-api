/*
 * Copyright (C) 2013 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the Lesser GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * Lesser GNU General Public License for more details.
 *
 * You should have received a copy of the Lesser GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Michi Henning <michi.henning@canonical.com>
 */

#include <scopes/Reply.h>
#include <scopes/ResultItem.h>

#include <scopes/internal/ReplyImpl.h>

namespace unity
{

namespace api
{

namespace scopes
{

//! @cond

Reply::Reply(internal::ReplyImpl* impl)
    : p(impl)
{
}

Reply::~Reply() noexcept
{
}

Category::SCPtr Reply::register_category(std::string const& id, std::string const& title, std::string const &icon, std::string const& renderer_template)
{
    return p->register_category(id, title, icon, renderer_template);
}

void Reply::register_category(Category::SCPtr category)
{
    return p->register_category(category);
}

Category::SCPtr Reply::lookup_category(std::string const& id) const
{
    return p->lookup_category(id);
}

bool Reply::push(ResultItem const& result) const
{
    return p->push(result);
}

void Reply::finished() const
{
    return p->finished();
}

//! @endcond

} // namespace scopes

} // namespace api

} // namespace unity

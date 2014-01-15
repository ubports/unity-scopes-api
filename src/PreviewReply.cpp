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

#include <scopes/PreviewReply.h>
#include <scopes/Result.h>
#include <scopes/PreviewWidget.h>

#include <scopes/internal/ReplyImpl.h>

namespace unity
{

namespace api
{

namespace scopes
{

//! @cond

PreviewReply::PreviewReply(internal::ReplyImpl* impl) :
    ObjectProxy(impl), ReplyBase(impl)
{
}

PreviewReply::~PreviewReply() noexcept
{
}

bool PreviewReply::push(PreviewWidgetList const& widgets) const
{
    return fwd()->push(widgets);
}

bool PreviewReply::push(std::string const& key, Variant const& value) const
{
    return fwd()->push(key, value);
}

//! @endcond

} // namespace scopes

} // namespace api

} // namespace unity

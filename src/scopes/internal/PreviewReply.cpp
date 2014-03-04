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

#include <unity/scopes/internal/PreviewReply.h>

#include <unity/scopes/internal/ReplyImpl.h>
#include <unity/scopes/PreviewWidget.h>
#include <unity/scopes/Result.h>

namespace unity
{

namespace scopes
{

namespace internal
{

//! @cond

PreviewReply::PreviewReply(internal::ReplyImpl* impl) : Reply(impl)
{
}

PreviewReply::~PreviewReply()
{
}

bool PreviewReply::register_layout(ColumnLayoutList const& layouts) const
{
    return fwd()->register_layout(layouts);
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

} // namespace internal

} // namespace scopes

} // namespace unity

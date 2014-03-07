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

#include <unity/scopes/internal/PreviewReplyImpl.h>

#include <unity/scopes/internal/MWReply.h>

using namespace std;

namespace unity
{

namespace scopes
{

namespace internal
{

PreviewReplyImpl::PreviewReplyImpl(MWReplyProxy const& mw_proxy, shared_ptr<QueryObjectBase> const& qo) :
    ObjectImpl(mw_proxy),
    ReplyImpl(mw_proxy, qo)
{
}

PreviewReplyImpl::~PreviewReplyImpl()
{
}

bool PreviewReplyImpl::register_layout(unity::scopes::ColumnLayoutList const& layouts)
{
#if 0
    if (layouts_push_disallowed_)
    {
        throw unity::LogicException("Reply::register_layout(): column layouts can only be registered once and before pushing preview widgets");
    }

    // basic check for consistency of layouts
    try
    {
        ColumnLayoutImpl::validate_layouts(layouts);
    }
    catch (unity::LogicException const &e)
    {
        throw unity::LogicException("Reply::register_layout(): Failed to validate layouts");
    }

    VariantMap vm;
    VariantArray arr;
    for (auto const& layout: layouts)
    {
        arr.push_back(Variant(layout.serialize()));
    }
    vm["columns"] = arr;
    return layouts_push_disallowed_ = push(vm);
#else
    return ReplyImpl::register_layout(layouts);
#endif
}

bool PreviewReplyImpl::push(unity::scopes::PreviewWidgetList const& widgets)
{
#if 0
    layouts_push_disallowed_ = true;

    VariantMap vm;
    VariantArray arr;
    for (auto const& widget : widgets)
    {
        arr.push_back(Variant(widget.serialize()));
    }
    vm["widgets"] = arr;
    return push(vm);
#else
    return ReplyImpl::push(widgets);
#endif
}

bool PreviewReplyImpl::push(string const& key, Variant const& value)
{
#if 0
    VariantMap vm;
    VariantMap nested;
    nested[key] = value;
    vm["preview-data"] = nested;
    return push(vm);
#else
    return ReplyImpl::push(key, value);
#endif
}

} // namespace internal

} // namespace scopes

} // namespace unity

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
 * Authored by: Pawel Stolowski <pawel.stolowski@canonical.com>
 *              Michi Henning <michi.henning@canonical.com>
 */

#include <unity/scopes/internal/PreviewReplyImpl.h>

#include <unity/scopes/internal/ColumnLayoutImpl.h>
#include <unity/scopes/internal/MWReply.h>
#include <unity/UnityExceptions.h>

using namespace std;

namespace unity
{

namespace scopes
{

namespace internal
{

PreviewReplyImpl::PreviewReplyImpl(MWReplyProxy const& mw_proxy, shared_ptr<QueryObjectBase> const& qo) :
    ObjectImpl(mw_proxy),
    ReplyImpl(mw_proxy, qo),
    layouts_push_disallowed_(false)
{
}

PreviewReplyImpl::~PreviewReplyImpl()
{
}

bool PreviewReplyImpl::register_layout(unity::scopes::ColumnLayoutList const& layouts)
{
    // basic check for consistency of layouts
    try
    {
        ColumnLayoutImpl::validate_layouts(layouts);
    }
    catch (unity::LogicException const &e)
    {
        throw unity::LogicException("Reply::register_layout(): Failed to validate layouts");
    }

    // OK to set this now because what follows can't go wrong unless the query is dead.
    if (layouts_push_disallowed_.exchange(true))
    {
        throw unity::LogicException("Reply::register_layout(): column layouts can only be registered "
                                    "once and before pushing preview widgets");
    }

    VariantMap vm;
    VariantArray arr;
    for (auto const& layout: layouts)
    {
        arr.push_back(Variant(layout.serialize()));
    }
    vm["columns"] = arr;
    if (!ReplyImpl::push(vm))
    {
        throw unity::LogicException("Reply::register_layout(): cannot register layout with finished "
                                    "or cancelled query");
    }
    return true;
}

bool PreviewReplyImpl::push(unity::scopes::PreviewWidgetList const& widgets)
{
    // TODO: why is it possible to push more than one list of widgets without getting an exception?
    layouts_push_disallowed_ = true;

    VariantMap vm;
    VariantArray arr;
    for (auto const& widget : widgets)
    {
        arr.push_back(Variant(widget.serialize()));
    }
    vm["widgets"] = arr;
    return ReplyImpl::push(vm);
}

bool PreviewReplyImpl::push(string const& key, Variant const& value)
{
    VariantMap vm;
    VariantMap nested;
    nested[key] = value;
    vm["preview-data"] = nested;
    return ReplyImpl::push(vm);
}

} // namespace internal

} // namespace scopes

} // namespace unity

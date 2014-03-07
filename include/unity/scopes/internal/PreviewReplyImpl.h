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

#ifndef UNITY_SCOPES_INTERNAL_PREVIEWREPLYIMPL_H
#define UNITY_SCOPES_INTERNAL_PREVIEWREPLYIMPL_H

#include <unity/scopes/ColumnLayout.h>
#include <unity/scopes/internal/MWReplyProxyFwd.h>
#include <unity/scopes/internal/ReplyImpl.h>
#include <unity/scopes/PreviewReply.h>
#include <unity/scopes/PreviewWidget.h>

namespace unity
{

namespace scopes
{

namespace internal
{

class QueryObjectBase;

class PreviewReplyImpl : public virtual unity::scopes::PreviewReply, public virtual ReplyImpl
{
public:
    PreviewReplyImpl(MWReplyProxy const& mw_proxy, std::shared_ptr<QueryObjectBase>const & qo);
    virtual ~PreviewReplyImpl();

    virtual bool register_layout(unity::scopes::ColumnLayoutList const& layouts) override;
    virtual bool push(unity::scopes::PreviewWidgetList const& widgets) override;
    virtual bool push(std::string const& key, Variant const& value) override;

private:
    std::atomic_bool layouts_push_disallowed_;
};

} // namespace internal

} // namespace scopes

} // namespace unity

#endif

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

#ifndef UNITY_SCOPES_PREVIEW_INTERNAL_REPLY_H
#define UNITY_SCOPES_PREVIEW_INTERNAL_REPLY_H

#include <unity/scopes/PreviewReply.h>

#include <unity/scopes/ColumnLayout.h>
#include <unity/scopes/internal/Reply.h>
#include <unity/scopes/ObjectProxy.h>
#include <unity/scopes/PreviewWidget.h>
#include <unity/scopes/ReplyProxyFwd.h>
#include <unity/scopes/Result.h>

namespace unity
{

namespace scopes
{

namespace internal
{
class QueryObject;
class ReplyImpl;

/**
\brief Allows the results of a query to be sent to the source of the query.
*/

class PreviewReply : public virtual unity::scopes::PreviewReply, public Reply
{
public:
    /// @cond
    PreviewReply(PreviewReply const&) = delete;
    /// @endcond

    /**
     \brief Registers a list of column layouts for current preview.

     Layouts need to be registered before pushing PreviewWidgetList, and must be
     registered only once.
     \return True if the query is still alive, false if the query failed or was cancelled.
     \throws unity::LogicException register_layout() is called more than once.
     */
    bool register_layout(ColumnLayoutList const& layouts) const override;

    /**
     \brief Sends widget definitions to the sender of the preview query.
     \throws unity::LogicException register_layout() is called more than once.
     */
    bool push(PreviewWidgetList const& widget_list) const override;
    /**
     \brief Sends data for a preview widget attribute.
     \throws unity::LogicException register_layout() is called more than once.
     */
    bool push(std::string const& key, Variant const& value) const override;

    /**
    \brief Destroys a Reply.

    If a Reply goes out of scope without a prior call to finished(), the destructor implicitly calls finished().
    */
    virtual ~PreviewReply();

protected:
    // @cond
    PreviewReply(ReplyImpl* impl);         // Instantiated only by ReplyImpl
    friend class ReplyImpl;
    // @endcond
};

} // namespace internal

} // namespace scopes

} // namespace unity

#endif

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

#ifndef UNITY_SCOPES_PREVIEW_REPLY_H
#define UNITY_SCOPES_PREVIEW_REPLY_H

#include <unity/scopes/ColumnLayout.h>
#include <unity/scopes/PreviewWidget.h>
#include <unity/scopes/Reply.h>

#include <string>

namespace unity
{
namespace scopes
{
/**
\brief Allows the results of a query to be sent to the source of the query.
*/
class PreviewReply : public virtual Reply
{
public:
    /// @cond
    virtual ~PreviewReply();
    PreviewReply(PreviewReply const&) = delete;
    /// @endcond

    /**
     \brief Registers a list of column layouts for current preview.

     Layouts need to be registered before pushing PreviewWidgetList, and must be
     registered only once.
     \return True if the query is still alive, false if the query failed or was cancelled.
     \throws unity::LogicException register_layout() is called more than once.
     */
    virtual bool register_layout(ColumnLayoutList const& layouts) const = 0;

    /**
     \brief Sends widget definitions to the sender of the preview query.
     \return True if the query is still alive, false if the query failed or was cancelled.
     */
    virtual bool push(PreviewWidgetList const& widget_list) const = 0;

    /**
     \brief Sends data for a preview widget attribute.
     \return True if the query is still alive, false if the query failed or was cancelled.
     */
    virtual bool push(std::string const& key, Variant const& value) const = 0;

protected:
    /// @cond
    PreviewReply();
    /// @endcond
};

} // namespace scopes

} // namespace unity

#endif

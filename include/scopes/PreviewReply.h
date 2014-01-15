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

#ifndef UNITY_API_SCOPES_PREVIEW_REPLY_H
#define UNITY_API_SCOPES_PREVIEW_REPLY_H

#include <scopes/ObjectProxy.h>
#include <scopes/ReplyProxyFwd.h>
#include <scopes/Result.h>
#include <scopes/PreviewWidget.h>
#include <scopes/Reply.h>

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{
class QueryObject;
class ReplyImpl;
}

/**
\brief Reply allows the results of a query to be sent to the source of the query.
*/

class UNITY_API PreviewReply : public virtual ReplyBase
{
public:
    PreviewReply(PreviewReply const&) = delete;

    bool push(PreviewWidgetList const& widget_list) const;

    bool push(std::string const& key, Variant const& value) const;

    /**
    \brief Destroys a Reply.
    If a Reply goes out of scope without a prior call to finished(), the destructor implicitly calls finished().
    */
    virtual ~PreviewReply() noexcept;

protected:
    PreviewReply(internal::ReplyImpl* impl);         // Instantiated only by ReplyImpl
    friend class internal::ReplyImpl;
};

} // namespace scopes

} // namespace api

} // namespace unity

#endif

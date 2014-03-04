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

#ifndef UNITY_SCOPES_REPLYPROXY_H
#define UNITY_SCOPES_REPLYPROXY_H

#include <memory>

namespace unity
{

namespace scopes
{

class Reply;
class SearchReply;
class PreviewReply;

/** \typedef ReplyBaseProxy
\brief Convenience type definition.
*/
typedef std::shared_ptr<Reply> ReplyBaseProxy;

/** \typedef SearchReplyProxy
\brief Convenience type definition.
*/
typedef std::shared_ptr<SearchReply> SearchReplyProxy;

/** \typedef PreviewReplyProxy
\brief Convenience type definition.
*/
typedef std::shared_ptr<PreviewReply> PreviewReplyProxy;

} // namespace scopes

} // namespace unity

#endif

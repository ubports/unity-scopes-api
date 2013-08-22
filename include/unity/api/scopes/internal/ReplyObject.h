/*
 * Copyright (C) 2013 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Michi Henning <michi.henning@canonical.com>
 */

#ifndef UNITY_API_REPLYS_INTERNAL_REPLYOBJECT_H
#define UNITY_API_REPLYS_INTERNAL_REPLYOBJECT_H

#include <unity/api/scopes/internal/AbstractObject.h>
#include <unity/api/scopes/ReplyBase.h>

#include <atomic>
#include <condition_variable>

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

// A ReplyObject sits in between the incoming requests from the middleware layer and the
// ReplyBase-derived implementation provided by the scope.
// This allows us to intercept all replies.

class ReplyObject final : public AbstractObject
{
public:
    UNITY_DEFINES_PTRS(ReplyObject);

    ReplyObject(ReplyBase::SPtr const& reply_base);
    virtual ~ReplyObject() noexcept;

    // Remote operation implementations
    void push(std::string const& result);
    void finished();

private:
    ReplyBase::SPtr reply_base_;
    std::atomic_bool finished_;
    std::mutex mutex_;
    std::condition_variable idle_;
    int num_push_;
};

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

#endif

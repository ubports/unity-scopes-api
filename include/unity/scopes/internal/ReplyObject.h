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

#ifndef UNITY_SCOPES_INTERNAL_REPLYOBJECT_H
#define UNITY_SCOPES_INTERNAL_REPLYOBJECT_H

#include <unity/scopes/internal/ReplyObjectBase.h>
#include <unity/scopes/internal/Reaper.h>
#include <unity/scopes/ListenerBase.h>
#include <unity/scopes/Variant.h>

#include <atomic>
#include <condition_variable>

namespace unity
{

namespace scopes
{

namespace internal
{

class RuntimeImpl;

// A ReplyObject sits in between the incoming requests from the middleware layer and the
// ListenerBase-derived implementation provided by the scope.
// This allows us to intercept all replies.

class ReplyObject : public ReplyObjectBase
{
public:
    UNITY_DEFINES_PTRS(ReplyObject);

    ReplyObject(ListenerBase::SPtr const& receiver_base, RuntimeImpl const* runtime, std::string const& scope_proxy);
    virtual ~ReplyObject();

    virtual bool process_data(VariantMap const& data) = 0;

    std::string origin_proxy() const;

    // Remote operation implementations
    void push(VariantMap const& result) noexcept override;
    void finished(ListenerBase::Reason reason, std::string const& error_message) noexcept override;

private:
    ListenerBase::SPtr const listener_base_;
    ReapItem::SPtr reap_item_;
    std::atomic_bool finished_;
    std::mutex mutex_;
    std::condition_variable idle_;
    std::string origin_proxy_;
    int num_push_;
};

} // namespace internal

} // namespace scopes

} // namespace unity

#endif

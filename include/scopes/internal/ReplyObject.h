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

#include <scopes/internal/AbstractObject.h>
#include <scopes/internal/Reaper.h>
#include <scopes/internal/CategoryRegistry.h>
#include <scopes/ReceiverBase.h>
#include <scopes/Variant.h>

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

class RuntimeImpl;

// A ReplyObject sits in between the incoming requests from the middleware layer and the
// ReceiverBase-derived implementation provided by the scope.
// This allows us to intercept all replies.

class ReplyObject final : public AbstractObject
{
public:
    UNITY_DEFINES_PTRS(ReplyObject);

    ReplyObject(ReceiverBase::SPtr const& reply_base, RuntimeImpl const* runtime);
    virtual ~ReplyObject() noexcept;

    // Remote operation implementations
    void push(VariantMap const& result) noexcept;
    void finished() noexcept;

private:
    ReceiverBase::SPtr const reply_base_;
    ReapItem::SPtr reap_item_;
    std::shared_ptr<CategoryRegistry> cat_registry_;
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

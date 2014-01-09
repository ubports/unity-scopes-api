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

#ifndef UNITY_API_REPLYS_INTERNAL_REPLYOBJECT_H
#define UNITY_API_REPLYS_INTERNAL_REPLYOBJECT_H

#include <scopes/internal/AbstractObject.h>
#include <scopes/internal/Reaper.h>
#include <scopes/internal/CategoryRegistry.h>
#include <scopes/ListenerBase.h>
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
// ListenerBase-derived implementation provided by the scope.
// This allows us to intercept all replies.

class ReplyObject : public AbstractObject
{
public:
    UNITY_DEFINES_PTRS(ReplyObject);

    ReplyObject(ListenerBase::SPtr const& receiver_base, RuntimeImpl const* runtime, std::string const& scope_name);
    virtual ~ReplyObject() noexcept;

    virtual void process_data(VariantMap const& data) = 0;

    // Remote operation implementations
    void push(VariantMap const& result) noexcept;
    void finished(ListenerBase::Reason reason, std::string const& error_message) noexcept;
    std::string origin_scope_name() const;

private:
    ListenerBase::SPtr const listener_base_;
    ReapItem::SPtr reap_item_;
    std::atomic_bool finished_;
    std::mutex mutex_;
    std::condition_variable idle_;
    std::string origin_scope_name_;
    int num_push_;
};

class ResultReplyObject : public ReplyObject
{
public:
    ResultReplyObject(SearchListener::SPtr const& receiver, RuntimeImpl const* runtime, std::string const& scope_name);
    virtual ~ResultReplyObject() noexcept;

    virtual void process_data(VariantMap const& data) override;

private:
    SearchListener::SPtr const receiver_;
    std::shared_ptr<CategoryRegistry> cat_registry_;
};

class ActivationReplyObject : public ReplyObject
{
public:
    ActivationReplyObject(ActivationListener::SPtr const& receiver, RuntimeImpl const* runtime, std::string const& scope_name);
    virtual void process_data(VariantMap const& data) override;

private:
    ActivationListener::SPtr const receiver_;
};

class PreviewReplyObject : public ReplyObject
{
public:
    PreviewReplyObject(PreviewListener::SPtr const& receiver, RuntimeImpl const* runtime, std::string const& scope_name);
    virtual ~PreviewReplyObject() noexcept;

    virtual void process_data(VariantMap const& data) override;

private:
    PreviewListener::SPtr const receiver_;
};

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

#endif

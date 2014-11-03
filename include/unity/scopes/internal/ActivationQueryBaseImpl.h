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
 */

#pragma once

#include <unity/scopes/internal/QueryBaseImpl.h>
#include <unity/scopes/ActivationResponse.h>
#include <unity/scopes/Result.h>
#include <unity/scopes/ActionMetadata.h>
#include <unity/util/NonCopyable.h>

namespace unity
{

namespace scopes
{

namespace internal
{

class ActivationQueryBaseImpl : public QueryBaseImpl
{
public:
    NONCOPYABLE(ActivationQueryBaseImpl);

    ActivationQueryBaseImpl(Result const& result, ActionMetadata const& metadata, std::string const& widget_id = "", std::string const& action_id = "");
    ~ActivationQueryBaseImpl() = default;
    ActivationResponse activate();

    Result result() const;
    ActionMetadata action_metadata() const;
    std::string widget_id() const;
    std::string action_id() const;

    void cancel() override;
    bool valid() const override;

private:
    Result result_;
    const ActionMetadata metadata_;
    const std::string widget_id_;
    const std::string action_id_;
    bool valid_;
    mutable std::mutex mutex_;
};

} // namespace internal

} // namespace scopes

} // namespace unity

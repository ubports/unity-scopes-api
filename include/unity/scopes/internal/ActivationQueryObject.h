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

#include <unity/scopes/internal/MWQueryCtrlProxyFwd.h>
#include <unity/scopes/internal/QueryObject.h>

namespace unity
{

namespace scopes
{
class ActivationQueryBase;

namespace internal
{

class ActivationQueryObject final : public QueryObject
{
public:
    UNITY_DEFINES_PTRS(ActivationQueryObject);

    ActivationQueryObject(std::shared_ptr<ActivationQueryBase> const& act_base,
                          MWReplyProxy const& reply,
                          MWQueryCtrlProxy const& ctrl,
                          boost::log::sources::severity_channel_logger_mt<>& logger);
    virtual ~ActivationQueryObject();
    virtual void run(MWReplyProxy const& reply, InvokeInfo const& info) noexcept override;

private:
    std::shared_ptr<ActivationQueryBase> act_base_;
};

} // namespace internal

} // namespace scopes

} // namespace unity

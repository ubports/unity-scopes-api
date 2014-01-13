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

#ifndef UNITY_INTERNAL_ACTIVATIONQUERYOBJECT_H
#define UNITY_INTERNAL_ACTIVATIONQUERYOBJECT_H

#include <scopes/internal/QueryObject.h>
#include <scopes/internal/MWQueryCtrlProxyFwd.h>

namespace unity
{

namespace api
{

namespace scopes
{
class ActivationBase;

namespace internal
{

class ActivationQueryObject final : public QueryObjectBase
{
public:
    UNITY_DEFINES_PTRS(ActivationQueryObject);

    ActivationQueryObject(std::shared_ptr<ActivationBase> const& act_base, MWReplyProxy const& reply, MWQueryCtrlProxy const& ctrl);
    virtual ~ActivationQueryObject() noexcept;
    void run(MWReplyProxy const& reply) noexcept override;
    void cancel() override;

    // Called by create_query(), to hold the reference count high until the run call arrives via the middleware,
    // and we can pass the shared_ptr to the ReplyImpl.
    void set_self(SPtr const& self);

private:
    std::shared_ptr<ActivationBase> act_base_;
    MWReplyProxy reply_;
    MWQueryCtrlProxy const ctrl_;
    SPtr self_;
};

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

#endif

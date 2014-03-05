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

#ifndef UNITY_SCOPES_ACTIVATIONQUERYBASE_H
#define UNITY_SCOPES_ACTIVATIONQUERYBASE_H

#include <unity/scopes/QueryBase.h>
#include <unity/util/DefinesPtrs.h>
#include <unity/scopes/ActivationResponse.h>

namespace unity
{

namespace scopes
{

namespace internal
{
class ActivationQueryBaseImpl;
}

/**
\brief Base class for an activation request that is executed inside a scope.

The default implementation responds with ActivationResponse(ActivationResponse::Status::NotHandled).
Reimplement this class and return an instance in ScopeBase::activate method for custom activation handling.
*/

class ActivationQueryBase : public QueryBase
{
public:
    /// @cond
    UNITY_DEFINES_PTRS(ActivationQueryBase);
    ActivationQueryBase();
    virtual ~ActivationQueryBase();
    /// @endcond

    /**
     \brief Called when the originator of the activation request cancelled it.
     */
    virtual void cancelled() override;

    /**
     \brief Return response to the activation request.

     The response is received by the originator of the request. Default implementation
     returns ActivationResponse(ActivationResponse::Status::NotHandled.
     \return The response to the activation request.
     */
    virtual ActivationResponse activate();

private:
    std::unique_ptr<internal::ActivationQueryBaseImpl> p;
};

} // namespace scopes

} // namespace unity

#endif

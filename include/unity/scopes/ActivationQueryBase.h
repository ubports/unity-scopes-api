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

class Result;
class ActionMetadata;

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
    virtual ~ActivationQueryBase();
    /// @endcond

    /**
     \brief Create ActivationQueryBase.

     \param result The result received in ScopeBase::activate().
     \param metadata The metadata received in ScopeBase::activate().
    */
    ActivationQueryBase(Result const& result, ActionMetadata const& metadata);

    /**
     \brief Create ActivationQueryBase.

     \param result The result received in ScopeBase::perform_action().
     \param metadata The metadata received in ScopeBase::perform_action().
     \param widget_id The widget identifier receiver in ScopeBase::perform_action().
     \param action_id The action identifier receiver in ScopeBase::perform_action().
    */
    ActivationQueryBase(Result const& result, ActionMetadata const& metadata, std::string const& widget_id, std::string const& action_id);

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

    /**
    \brief Get the result for this activation request handler.

    \return The result passed to the constructor of this object.
    */
    Result result() const;

    /**
    \brief Get the metadata for this activation request handler.

    \return The metadata passed to the constructor of this object.
    */
    ActionMetadata action_metadata() const;

    /**
    \brief Get the widget identifier for this activation request handler.

    Widget identifier is empty when using the
    unity::scopes::ActivationQueryBase(Result const& result, ActionMetadata const& metadata) constructor.

    \return The widget identifier passed to the constructor of this object.
    */
    std::string widget_id() const;

    /**
    \brief Get the action identifier for this activation request handler.

    Action identifier is empty when using the
    unity::scopes::ActivationQueryBase(Result const& result, ActionMetadata const& metadata) constructor.

    \return The action identifier passed to the constructor of this object.
    */
    std::string action_id() const;

private:
    std::unique_ptr<internal::ActivationQueryBaseImpl> p;
};

} // namespace scopes

} // namespace unity

#endif

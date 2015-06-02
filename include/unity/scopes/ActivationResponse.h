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

#include <unity/scopes/Variant.h>
#include <unity/scopes/CannedQuery.h>
#include <unity/scopes/Result.h>
#include <memory>

namespace unity
{

namespace scopes
{

namespace internal
{
class ActivationResponseImpl;
}

/**
\brief Response to a result activation.
*/

class ActivationResponse final
{
public:
    /**
     \brief Status of a unity::scopes::ScopeBase::activate, unity::scopes::ScopeBase::perform_action,
     or unity::scopes::ScopeBase::activate_result_action request.
     */
    enum Status
    {
        NotHandled,   /**< Activation of this result wasn't handled by the scope */
        ShowDash,     /**< Activation of this result was handled, show the Dash */
        HideDash,     /**< Activation of this result was handled, hide the Dash */
        ShowPreview,  /**< Preview should be requested for this result */
        PerformQuery, /**< Perform new search. This state is implied if creating ActivationResponse with CannedQuery object and is invalid otherwise */
        UpdateResult, /**< Update the result. This state is implied if creating ActivationResponse with Result object and is invalid otherwise */
    };

    /**
    \brief Creates ActivationResponse with given status.
    \param status The activation status.
    \throws unity::InvalidArgumentException if status is Status::PerformQuery or Status::Update.
    To create an ActivationResponse for one of these types, use the ActivationResponse(CannedQuery const&)
    or ActivationResponse(Result const&) constructors.
    */
    ActivationResponse(Status status);

    /**
    \brief Creates an ActivationResponse with status Status::PerformQuery and a search query to be executed.
    \param query The search query to be executed by the client.
     */
    ActivationResponse(CannedQuery const& query);

    /**
    \brief Creates an ActivationResponse with status Status::UpdateResult and an updated result that
    replaces the original result of the action.
    \param updated_result The updated result to replace the original result of the action.
     */
    ActivationResponse(Result const& updated_result);

    /**@name Copy and assignment
    Copy and assignment operators (move and non-move versions) have the usual value semantics.
    */
    //{@
    ActivationResponse(ActivationResponse const& other);
    ActivationResponse(ActivationResponse&& other);

    ActivationResponse& operator=(ActivationResponse const& other);
    ActivationResponse& operator=(ActivationResponse&& other);
    //@}

    /**
    \brief Get activation status.
    \return The activation status.
    */
    ActivationResponse::Status status() const;

    /**
     \brief Attach arbitrary data to this response.

     The attached data is sent back to the scope if the status of this response is Status::ShowPreview.
     \param data An arbitrary value attached to the response.
     */
    void set_scope_data(Variant const& data);

    /**
     \brief Get data attached to this response object.
     \return The data attached to the response.
     */
    Variant scope_data() const;

    /**
     \brief A query to be executed if status is Status::PerformQuery.

     \throws unity::LogicException if the status of this ActivationResponse is anything other than Status::PerformQuery.
     \return The query to be executed by the client.
    */
    CannedQuery query() const;

    /**
     \brief The updated result if status is Status::UpdateResult.

     \throws unity::LogicException if the status of this ActivationResponse is anything other than Status::UpdateResult.
     \return The result to be displayed instead of the original result.
    */
    Result updated_result() const;

    /// @cond
    VariantMap serialize() const;

    ~ActivationResponse();
    /// @endcond

private:
    std::unique_ptr<internal::ActivationResponseImpl> p;
    ActivationResponse(internal::ActivationResponseImpl* pimpl);
    friend class internal::ActivationResponseImpl;
};

} // namespace scopes

} // namespace unity

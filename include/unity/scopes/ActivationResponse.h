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

#ifndef UNITY_SCOPES_ACTIVATIONRESPONSE_H
#define UNITY_SCOPES_ACTIVATIONRESPONSE_H

#include <unity/scopes/Variant.h>
#include <unity/scopes/Query.h>
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
\brief Carries response to a Result activation request.
*/
class ActivationResponse final
{
public:
    /*!
     \brief Status of an unity::scopes::ScopeBase::activate or unity::scopes::ScopeBase::perform_action request.
     */
    enum Status
    {
        NotHandled,  /**< Activation of this result wasn't handled by the scope */
        ShowDash,    /**< Activation of this result was handled, show the Dash */
        HideDash,    /**< Activation of this result was handled, hide the Dash */
        ShowPreview, /**< Preview should be requested for this result */
        PerformQuery /**< Perform new search. This state is implied if creating ActivationResponse with Query object and is invalid otherwise */
    };

    /**
    \brief Creates ActivationResponse with given status.
    Throws unity::InvalidArgumentException if status is Status::PerformQuery - to
    create ActivationResponse of that type, use ActivationResponse(Query const&)
    constructor.
    \param status activation status
    */
    ActivationResponse(Status status);

    /**
    \brief Creates ActivationResponse with activation status of Status::PerformQuery and a search query to be executed.
    \param query search query to be executed by client
     */
    ActivationResponse(Query const& query);

    /// @cond
    ~ActivationResponse();
    ActivationResponse(ActivationResponse const& other);
    ActivationResponse(ActivationResponse&& other);
    ActivationResponse& operator=(ActivationResponse const& other);
    ActivationResponse& operator=(ActivationResponse&& other);
    /// @endcond

    /**
    \brief Get activation status.
    \return activation status
    */
    ActivationResponse::Status status() const;

    /**
     \deprecated Attach arbitrary data to this response.  This method will be removed in version 0.4.0, please use set_scope_data instead.
     */
    void setHints(VariantMap const& hints);

    /**
     \brief Attach arbitrary data to this response.

     The attached data will be sent back to the scope if status of this response is Status::ShowPreview.
     \param data arbitrary value attached to response
     */
    void set_scope_data(Variant const& data);

    /**
     \deprecated Get data attached to this response object. This method will be removed in version 0.4.0, please use scope_data instead.

     This method returns data attached with setHints() or set_scope_data() call; this method returns empty VariantMap if the attached data was added with
     set_scope_data() and it is is not of VariantMap type.

     \return data attached to response
     */
    VariantMap hints() const;

    /**
     \brief Get data attached to this response object.
     \return data attached to response
     */
    Variant scope_data() const;

    /**
     \brief Query to be executed if status is Status::PerformQuery.

     This method throws unity::LogicException is status of this ActivationResponse is different than Status::PerformQuery.
     \return query to be executed by client.
    */
    Query query() const;

    /// @cond
    VariantMap serialize() const;
    /// @endcond

private:
    std::shared_ptr<internal::ActivationResponseImpl> p;
    ActivationResponse(std::shared_ptr<internal::ActivationResponseImpl> pimpl);
    friend class internal::ActivationResponseImpl;
};

} // namespace scopes

} // namespace unity

#endif

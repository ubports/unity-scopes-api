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

#ifndef UNITY_SCOPES_ACTIVATIONLISTENERBASE_H
#define UNITY_SCOPES_ACTIVATIONLISTENERBASE_H

#include <unity/scopes/ListenerBase.h>
#include <unity/util/NonCopyable.h>

namespace unity
{

namespace scopes
{
class ActivationResponse;

/**
\brief Base class to receive a response to a result activation request.
*/

class ActivationListenerBase: public ListenerBase
{
public:
    /// @cond
    NONCOPYABLE(ActivationListenerBase);
    UNITY_DEFINES_PTRS(ActivationListenerBase);

    ~ActivationListenerBase();
    /// @endcond

    /**
    \brief Called once by the scopes run time with the activation response.

    Default implementation does nothing.
    \param response response to the activation request, returned by scope from ActivationQueryBase::activate().
    */
    virtual void activated(ActivationResponse const& response);

    /**
    \brief Called once by the scopes run time after receiving activation response.

    Exceptions thrown from finished() are ignored.
    Default implementation does nothing.
    \param r Indicates the cause for the call to finished().
    \param error_message If r is set to Reason::Error, error_message contains further details.
           Otherwise, error_message is the empty string.
     */
    virtual void finished(Reason r, std::string const& error_message) override;

protected:
    /// @cond
    ActivationListenerBase();
    /// @endcond
};

} // namespace scopes

} // namespace unity

#endif

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

#ifndef UNITY_SCOPES_XXX_H
#define UNITY_SCOPES_XXX_H

#include <unity/SymbolExport.h>
#include <scopes/ListenerBase.h>

namespace unity
{

namespace api
{

namespace scopes
{
class ActivationResponse;

/**
\brief
*/
class UNITY_API ActivationListener: public ListenerBase
{
public:
    /// @cond
    NONCOPYABLE(ActivationListener);
    UNITY_DEFINES_PTRS(ActivationListener);

    ~ActivationListener() noexcept;
    /// @endcond

    /**
    \brief TODO
    */
    virtual void activation_response(ActivationResponse const& response);
    void finished(Reason r, std::string const& error_message) override;

protected:
    /// @cond
    ActivationListener();
    /// @endcond
};

} // namespace scopes

} // namespace api

} // namespace unity

#endif

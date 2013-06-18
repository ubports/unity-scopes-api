/*
 * Copyright (C) 2013 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Michi Henning <michi.henning@canonical.com>
 */

#ifndef UNITY_API_SCOPES_SCOPEEXCEPTIONS_H
#define UNITY_API_SCOPES_SCOPEEXCEPTIONS_H

#include <unity/Exception.h>

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{
class MiddlewareExceptionImpl;
}

/**
\brief Exception to indicate that something went wrong with the middleware layer.
*/

class UNITY_API MiddlewareException : public unity::Exception
{
public:
    /**
    \brief Constructs the exception.
    \param reason Further details about the cause of the exception.
    */
    explicit MiddlewareException(std::string const& reason);
    //! @cond
    MiddlewareException(MiddlewareException const&);
    MiddlewareException& operator=(MiddlewareException const&);
    virtual ~MiddlewareException() noexcept;
    //! @endcond

    /**
    \brief Returns the fully-qualified name of the exception.
    */
    virtual char const* what() const noexcept override;

    /**
    \brief Returns a <code>std::exception_ptr</code> to <code>this</code>.
    */
    virtual std::exception_ptr self() const override;
};

} // namespace scopes

} // namespace api

} // namespace unity

#endif

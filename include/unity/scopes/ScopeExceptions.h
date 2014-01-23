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
 * Authored by: Michi Henning <michi.henning@canonical.com>
 */

#ifndef UNITY_SCOPES_SCOPEEXCEPTIONS_H
#define UNITY_SCOPES_SCOPEEXCEPTIONS_H

#include <unity/Exception.h>

namespace unity
{

namespace scopes
{

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
    virtual ~MiddlewareException();
    //! @endcond

    /**
    \brief Returns a <code>std::exception_ptr</code> to <code>this</code>.
    */
    virtual std::exception_ptr self() const override;
};

/**
\brief Exception to indicate that something went wrong with the contents of configuration files.
*/

class UNITY_API ConfigException : public unity::Exception
{
public:
    /**
    \brief Constructs the exception.
    \param reason Further details about the cause of the exception.
    */
    explicit ConfigException(std::string const& reason);
    //! @cond
    ConfigException(ConfigException const&);
    ConfigException& operator=(ConfigException const&);
    virtual ~ConfigException();
    //! @endcond

    /**
    \brief Returns a <code>std::exception_ptr</code> to <code>this</code>.
    */
    virtual std::exception_ptr self() const override;
};

/**
\brief Exception to indicate that an object wasn't found by a lookup function.
*/

class UNITY_API NotFoundException : public unity::Exception
{
public:
    /**
    \brief Constructs the exception.
    \param reason Further details about the cause of the exception.
    \param name The name of the object that was not found.
    */
    explicit NotFoundException(std::string const& reason, std::string const& name);
    //! @cond
    NotFoundException(NotFoundException const&);
    NotFoundException& operator=(NotFoundException const&);
    virtual ~NotFoundException();
    //! @endcond

    /**
    \brief Returns a <code>std::exception_ptr</code> to <code>this</code>.
    */
    virtual std::exception_ptr self() const override;

    /**
    \brief Returns the name that was passed to the constructor.
    */
    virtual std::string name() const;

private:
    std::string name_;
};

} // namespace scopes

} // namespace unity

#endif

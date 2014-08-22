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

class MiddlewareException : public virtual unity::Exception
{
public:
    /**
    \brief Constructs the exception.
    \param reason Further details about the cause of the exception.
    */
    explicit MiddlewareException(std::string const& reason);

    /**@name Copy and assignment
    Copy constructor and assignment operator have the usual value semantics.
    */
    //{@
    MiddlewareException(MiddlewareException const&);
    MiddlewareException& operator=(MiddlewareException const&);
    //@}

    /// @cond
    virtual ~MiddlewareException();
    /// @endcond

    /**
    \brief Returns a <code>std::exception_ptr</code> to <code>this</code>.
    \return <code>std::exception_ptr</code> to <code>this</code>.
    */
    virtual std::exception_ptr self() const override;
};

/**
\brief Exception to indicate that a (twoway) request was sent to an
       object with an unknown identity.
*/

class ObjectNotExistException : public virtual MiddlewareException
{
public:
    /**
    \brief Constructs the exception.
    \param reason Further details about the cause of the exception.
    \param id The identity of the unknown object.
    */
    explicit ObjectNotExistException(std::string const& reason, std::string const& id);

    /**@name Copy and assignment
    Copy constructor and assignment operator have the usual value semantics.
    */
    //{@
    ObjectNotExistException(ObjectNotExistException const&);
    ObjectNotExistException& operator=(ObjectNotExistException const&);
    //@}

    /// @cond
    virtual ~ObjectNotExistException();
    /// @endcond

    /**
    \brief Returns a <code>std::exception_ptr</code> to <code>this</code>.
    \return <code>std::exception_ptr</code> to <code>this</code>.
    */
    virtual std::exception_ptr self() const override;

    /**
    \brief Returns the identity of the non-existent object.
    \return The identity of the non-existent object.
    */
    virtual std::string id() const;

private:
    std::string id_;
};

/**
\brief Exception to indicate that a twoway request timed out.
*/

class TimeoutException : public virtual MiddlewareException
{
public:
    /**
    \brief Constructs the exception.
    \param reason Further details about the cause of the exception.
    */
    explicit TimeoutException(std::string const& reason);

    /**@name Copy and assignment
    Copy constructor and assignment operator have the usual value semantics.
    */
    //{@
    TimeoutException(TimeoutException const&);
    TimeoutException& operator=(TimeoutException const&);
    //@}

    /// @cond
    virtual ~TimeoutException();
    /// @endcond

    /**
    \brief Returns a <code>std::exception_ptr</code> to <code>this</code>.
    \return <code>std::exception_ptr</code> to <code>this</code>.
    */
    virtual std::exception_ptr self() const override;
};

/**
\brief Exception to indicate that something went wrong with the contents of configuration files.
*/

class ConfigException : public virtual unity::Exception
{
public:
    /**
    \brief Constructs the exception.
    \param reason Further details about the cause of the exception.
    */
    explicit ConfigException(std::string const& reason);

    /**@name Copy and assignment
    Copy constructor and assignment operator have the usual value semantics.
    */
    //{@
    ConfigException(ConfigException const&);
    ConfigException& operator=(ConfigException const&);
    //@}

    /// @cond
    virtual ~ConfigException();
    /// @endcond

    /**
    \brief Returns a <code>std::exception_ptr</code> to <code>this</code>.
    \return <code>std::exception_ptr</code> to <code>this</code>.
    */
    virtual std::exception_ptr self() const override;
};

/**
\brief Exception to indicate that an object wasn't found by a lookup function.
*/

class NotFoundException : public virtual unity::Exception
{
public:
    /**
    \brief Constructs the exception.
    \param reason Further details about the cause of the exception.
    \param name The name of the object that was not found.
    */
    explicit NotFoundException(std::string const& reason, std::string const& name);

    /**@name Copy and assignment
    Copy constructor and assignment operator have the usual value semantics.
    */
    //{@
    NotFoundException(NotFoundException const&);
    NotFoundException& operator=(NotFoundException const&);
    //@}

    /// @cond
    virtual ~NotFoundException();
    /// @endcond

    /**
    \brief Returns a <code>std::exception_ptr</code> to <code>this</code>.
    \return <code>std::exception_ptr</code> to <code>this</code>.
    */
    virtual std::exception_ptr self() const override;

    /**
    \brief Returns the name that was passed to the constructor.
    \return The name that was passed to the constructor.
    */
    virtual std::string name() const;

private:
    std::string name_;
};

} // namespace scopes

} // namespace unity

#endif

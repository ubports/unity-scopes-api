/*
 * Copyright (C) 2014 Canonical Ltd
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
 * Authored by: Marcus Tomlinson <marcus.tomlinson@canonical.com>
 */

#ifndef UNITY_SCOPES_QUERYMETADATA_H
#define UNITY_SCOPES_QUERYMETADATA_H

#include <unity/scopes/Variant.h>
#include <string>
#include <memory>

namespace unity
{

namespace scopes
{

namespace internal
{

class QueryMetadataImpl;

}

/**
\brief Base class for extra metadata passed to scopes as a part of a request.
*/
class QueryMetadata
{
public:
    /**@name Copy and assignment
    Copy and assignment operators (move and non-move versions) have the usual value semantics.
    */
    //{@
    QueryMetadata(QueryMetadata const& other);
    QueryMetadata(QueryMetadata&&);

    QueryMetadata& operator=(QueryMetadata const& other);
    QueryMetadata& operator=(QueryMetadata&&);
    //@}

    /// @cond
    virtual ~QueryMetadata();
    /// @endcond

    /**
    \brief Get the locale string.
    \return The locale string
    */
    std::string locale() const;

    /**
    \brief Get the form factor string.
    \return The form factor string
    */
    std::string form_factor() const;

    /**
    \brief Indicates the internet connectivity status.

    The `Unknown` enumerator indicates that set_internet_connectivity() has not yet been called,
    hense the connectivity status is currently unknown.

    The `Connected` enumerator simply indicates that we are currently connected to the internet.
    This does not necessarily mean that a particular host on the internet will be reachable.

    The `Disconnected` enumerator indicates that we are currently not connected to the internet.
    In this state, a scope need not waste any time attempting remote calls, as they will almost
    certainly fail.
    */
    enum ConnectivityStatus { Unknown, Connected, Disconnected };

    /**
    \brief Set internet connectivity status.
    \param connectivity_status Enum indicating the internet connectivity status.
    */
    void set_internet_connectivity(ConnectivityStatus connectivity_status);

    /**
    \brief Get internet connectivity status.
    \return Enum indicating the internet connectivity status.
    */
    ConnectivityStatus internet_connectivity() const;

    /// @cond
    VariantMap serialize() const;
    /// @endcond

protected:
    /// @cond
    std::unique_ptr<internal::QueryMetadataImpl> p;
    QueryMetadata(internal::QueryMetadataImpl* impl);
    /// @endcond
};

} // namespace scopes

} // namespace unity

#endif

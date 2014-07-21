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
 * Authored by: Pawel Stolowski <pawel.stolowski@canonical.com>
 */

#ifndef UNITY_SCOPES_ACTIONMETADATA_H
#define UNITY_SCOPES_ACTIONMETADATA_H

#include <unity/scopes/Variant.h>
#include <unity/util/DefinesPtrs.h>

namespace unity
{

namespace scopes
{

namespace internal
{
class ActionMetadataImpl;
}

/**
\brief Metadata passed to scopes for preview and activation.
\see unity::scopes::ScopeBase::preview, unity::scopes::ScopeBase::activate, unity::scopes::ScopeBase::perform_action
*/

class ActionMetadata final
{
public:
    /// @cond
    UNITY_DEFINES_PTRS(ActionMetadata);

    ~ActionMetadata();
    /// @endcond

    /**
    \brief Create ActionMetadata with the given locale and form factor.
    \param locale locale string, eg. en_EN
    \param form_factor form factor name, e.g. phone, desktop, phone-version etc.
    */
    ActionMetadata(std::string const& locale, std::string const& form_factor);

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
     \brief Attach arbitrary data to this ActionMetadata.
     \param data The data value to attach.
     */
    void set_scope_data(Variant const& data);

    /**
     \brief Get data attached to this ActionMetadata.
     \return The attached data, or Variant::null.
     */
    Variant scope_data() const;

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

    /**@name Copy and assignment
    Copy and assignment operators (move and non-move versions) have the usual value semantics.
    */
    //{@
    ActionMetadata(ActionMetadata const& other);
    ActionMetadata(ActionMetadata&&);

    ActionMetadata& operator=(ActionMetadata const& other);
    ActionMetadata& operator=(ActionMetadata&&);
    //@}

    /// @cond
    VariantMap serialize() const;
    /// @endcond

private:
    std::unique_ptr<internal::ActionMetadataImpl> p;

    ActionMetadata(internal::ActionMetadataImpl *impl);
    friend class internal::ActionMetadataImpl;
};

} // namespace scopes

} // namespace unity

#endif

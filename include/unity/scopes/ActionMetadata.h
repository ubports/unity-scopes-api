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

#include <unity/scopes/QueryMetadata.h>
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

class ActionMetadata : public QueryMetadata
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
     \brief Attach arbitrary data to this ActionMetadata.
     \param data The data value to attach.
     */
    void set_scope_data(Variant const& data);

    /**
     \brief Get data attached to this ActionMetadata.
     \return The attached data, or Variant::null.
     */
    Variant scope_data() const;

    /**@name Copy and assignment
    Copy and assignment operators (move and non-move versions) have the usual value semantics.
    */
    //{@
    ActionMetadata(ActionMetadata const& other);
    ActionMetadata(ActionMetadata&&);

    ActionMetadata& operator=(ActionMetadata const& other);
    ActionMetadata& operator=(ActionMetadata&&);
    //@}

    /**
    \brief Sets a hint.

    \param key The name of the hint.
    \param value Hint value
    */
    void set_hint(std::string const& key, Variant const& value);

    /**
    \brief Get all hints.

    \return Hints dictionary.
    \throws unity::NotFoundException if no hints are available.
    */
    VariantMap hints() const;

    /**
    \brief Check if this SearchMetadata has a hint.
    \param key The hint name.
    \return True if the hint is set.
    */
    bool contains_hint(std::string const& key) const;

    /**
    \brief Returns a reference to a hint.

    This method can be used to read or set hints. Setting a value of an existing hint overwrites
    its previous value.
    Referencing a non-existing hint automatically creates it with a default value of Variant::Type::Null.
    \param key The name of the hint.
    \return A reference to the hint.
    */
    Variant& operator[](std::string const& key);

    /**
    \brief Returns a const reference to a hint.

    This method can be used for read-only access to hints.
    Referencing a non-existing hint throws unity::InvalidArgumentException.
    \param key The name of the hint.
    \return A const reference to the hint.
    \throws unity::NotFoundException if no hint with the given name exists.
    */
    Variant const& operator[](std::string const& key) const;

private:
    ActionMetadata(internal::ActionMetadataImpl* impl);
    friend class internal::ActionMetadataImpl;
};

} // namespace scopes

} // namespace unity

#endif

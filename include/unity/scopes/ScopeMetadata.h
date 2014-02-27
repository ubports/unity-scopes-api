/*
 * Copyright (C) 2013 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the Lesser GNU General Public License version 3 as
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

#ifndef UNITY_SCOPES_SCOPEMETADATA_H
#define UNITY_SCOPES_SCOPEMETADATA_H

#include <unity/scopes/Scope.h>

namespace unity
{

namespace scopes
{

namespace internal
{
class ScopeMetadataImpl;
} // namespace internal

namespace testing
{
class ScopeMetadataBuilder;
} // namespace testing

/**
 \brief Holds scope attributes such as name, description, icon etc.
 The information carried by ScopeMetadata comes from the .ini file of given scope (for local scopes)
 or is fetched from the remote server (for scopes running on Smart Scopes Server).
 Use unity::scopes::Registry to get ScopeMetadata for a specific scope or all scopes.
 */

class ScopeMetadata final
{
public:
    /// @cond
    UNITY_DEFINES_PTRS(ScopeMetadata);

    ScopeMetadata(ScopeMetadata const& other);
    ScopeMetadata(ScopeMetadata&&);
    ~ScopeMetadata();

    ScopeMetadata& operator=(ScopeMetadata const& other);
    ScopeMetadata& operator=(ScopeMetadata&&);
    /// @endcond

    /**
     \brief Get the scope identifier.
     \return The name of the scope.
     */
    std::string scope_id() const;

    /**
     \brief Get the proxy object for this scope.
     \return The scope proxy.
     */
    ScopeProxy proxy() const;

    /**
     \brief Get the display name for this scope.
     \return The scope display name.
     */
    std::string display_name() const;   // localized

    /**
     \brief Get the description for this scope.
     \return The scope description.
     */
    std::string description() const;    // localized

    /**
     \brief Get the author for this scope.
     \return The scope author.
     */
    std::string author() const;

    /**
     \brief Get the art for this scope.
     \return The scope art.
     */
    std::string art() const;            // optional

    /**
     \brief Get the icon for this scope.
     \return The scope icon.
    */
    std::string icon() const;           // optional

    /**
     \brief Get the search hint for this scope.
     \return The search hint.
    */
    std::string search_hint() const;    // localized, optional

    /**
     \brief Get the hot key for this scope.
     \return The hot key.
     */
    std::string hot_key() const;        // localized, optional

    /**
     \brief Check if this scope should be hidden in the Dash.
     \return True if this scope is invisible.
     */
    bool invisible() const;             // optional (default = false)

    /**
       \brief Returns a dictionary of all attributes of this ScopeMetadata instance.
       \return Dictionary of all metadata attributes.
    */
    VariantMap serialize() const;

private:
    ScopeMetadata(std::unique_ptr<internal::ScopeMetadataImpl>);           // Instantiable only by ScopeMetadataImpl
    std::unique_ptr<internal::ScopeMetadataImpl> p;

    friend class internal::ScopeMetadataImpl;
};

} // namespace scopes

} // namespace unity

#endif

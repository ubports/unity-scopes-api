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

class ScopeMetadata final
{
public:
    UNITY_DEFINES_PTRS(ScopeMetadata);

    /// @cond
    ScopeMetadata(ScopeMetadata const& other);
    ScopeMetadata(ScopeMetadata&&);
    ~ScopeMetadata();

    ScopeMetadata& operator=(ScopeMetadata const& other);
    ScopeMetadata& operator=(ScopeMetadata&&);
    /// @endcond

    std::string scope_name() const;
    ScopeProxy proxy() const;
    std::string display_name() const;   // localized
    std::string description() const;    // localized
    std::string author() const;
    std::string art() const;            // optional
    std::string icon() const;           // optional
    std::string search_hint() const;    // localized, optional
    std::string hot_key() const;        // localized, optional
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

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

#ifndef UNITY_API_SCOPES_SCOPEMETADATA_H
#define UNITY_API_SCOPES_SCOPEMETADATA_H

#include <scopes/Scope.h>

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{
class ScopeMetadataImpl;
} // namespace internal

class UNITY_API ScopeMetadata final
{
public:
    ScopeMetadata(ScopeMetadata const& other);
    ScopeMetadata(ScopeMetadata&&);
    ~ScopeMetadata();

    ScopeMetadata& operator=(ScopeMetadata const& other);
    ScopeMetadata& operator=(ScopeMetadata&&);

    std::string scope_name() const;
    std::string icon_uri() const;
    ScopeProxy proxy() const;
    std::string localized_name() const; // localized
    std::string description() const;    // localized
    std::string search_hint() const;    // localized, optional
    std::string hot_key() const;        // localized, optional

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

} // namespace api

} // namespace unity

#endif

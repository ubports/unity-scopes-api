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

#include <unity/SymbolExport.h>
#include <unity/scopes/QueryMetadata.h>

namespace unity
{

namespace scopes
{

namespace internal
{
class ActionMetadataImpl;
}

/**
\brief
*/
class UNITY_API ActionMetadata : public QueryMetadata
{
public:
    ActionMetadata(std::string const& locale, std::string const& form_factor);
    void set_scope_data(Variant const& data);
    Variant scope_data() const;

    /// @cond
    ActionMetadata(ActionMetadata const& other);
    ActionMetadata(ActionMetadata&&);
    ~ActionMetadata();

    ActionMetadata& operator=(ActionMetadata const &other);
    ActionMetadata& operator=(ActionMetadata&&);
    /// @endcond

private:
    ActionMetadata(internal::ActionMetadataImpl *impl);
    internal::ActionMetadataImpl* fwd() const;
    friend class internal::ActionMetadataImpl;
};

} // namespace scopes

} // namespace unity

#endif

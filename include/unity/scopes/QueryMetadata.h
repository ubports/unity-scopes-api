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

#ifndef UNITY_SCOPES_QUERYMETADATA_H
#define UNITY_SCOPES_QUERYMETADATA_H

#include <unity/SymbolExport.h>
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
class UNITY_API QueryMetadata
{
public:
    QueryMetadata(QueryMetadata const& other);
    QueryMetadata(QueryMetadata&&);
    virtual ~QueryMetadata();

    QueryMetadata& operator=(QueryMetadata&&);

    std::string locale() const;
    std::string form_factor() const;

    VariantMap serialize() const;

protected:
    QueryMetadata(std::string const& locale, std::string const& form_factor);

private:
    QueryMetadata(internal::QueryMetadataImpl *impl);
    std::unique_ptr<internal::QueryMetadataImpl> p;

    friend class SearchMetadata;
    friend class ActionMetadata;
};

} // namespace scopes

} // namespace unity

#endif

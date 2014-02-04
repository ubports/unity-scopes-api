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

#ifndef UNITY_SCOPES_SEARCHMETADATA_H
#define UNITY_SCOPES_SEARCHMETADATA_H

#include <unity/SymbolExport.h>
#include <unity/scopes/QueryMetadata.h>

namespace unity
{

namespace scopes
{

namespace internal
{
class SearchMetadataImpl;
}

/**
\brief Metadata passed with search requests.
*/
class UNITY_API SearchMetadata : public QueryMetadata
{
public:
    SearchMetadata(std::string const& locale, std::string const& form_factor, GeoCoordinate const& location);
    SearchMetadata(int cardinality, std::string const& locale, std::string const& form_factor, GeoCoordinate const& location);

    /// @cond
    SearchMetadata(SearchMetadata const& other);
    SearchMetadata(SearchMetadata&&);
    ~SearchMetadata();

    SearchMetadata& operator=(SearchMetadata const &other);
    SearchMetadata& operator=(SearchMetadata&&);
    /// @endcond

    void set_cardinality(int cardinality);
    int cardinality() const;

private:
    SearchMetadata(internal::SearchMetadataImpl *impl);
    internal::SearchMetadataImpl* fwd() const;
    friend class internal::SearchMetadataImpl;
};

} // namespace scopes

} // namespace unity

#endif

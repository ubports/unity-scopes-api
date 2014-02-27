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

#include <unity/scopes/QueryMetadata.h>
#include <unity/util/DefinesPtrs.h>

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
class SearchMetadata : public QueryMetadata
{
public:
    /// @cond
    UNITY_DEFINES_PTRS(SearchMetadata);
    /// @endcond

    /**
     \brief Create SearchMetadata with given locale and form factor.
     \param locale locale string, eg. en_EN
     \param form_factor form factor name, e.g. phone, desktop, phone-version etc.
     */
    SearchMetadata(std::string const& locale, std::string const& form_factor);

    /**
     \brief Create SearchMetadata with given cardinality, locale and form factor.
     \param cardinality maximum number of search results
     \param locale locale string, eg. en_EN
     \param form_factor form factor name, e.g. phone, desktop, phone-version etc.
     */
    SearchMetadata(int cardinality, std::string const& locale, std::string const& form_factor);

    /// @cond
    SearchMetadata(SearchMetadata const& other);
    SearchMetadata(SearchMetadata&&);
    ~SearchMetadata();

    SearchMetadata& operator=(SearchMetadata const &other);
    SearchMetadata& operator=(SearchMetadata&&);
    /// @endcond

    /**
     \brief Set cardinality.
     \param cardinality The maximum number of search results.
    */
    void set_cardinality(int cardinality);

    /**
     \brief Get cardinality.
     \return The maxmium number of search results, or 0 for no limit.
    */
    int cardinality() const;

private:
    SearchMetadata(internal::SearchMetadataImpl *impl);
    internal::SearchMetadataImpl* fwd() const;
    friend class internal::SearchMetadataImpl;
};

} // namespace scopes

} // namespace unity

#endif

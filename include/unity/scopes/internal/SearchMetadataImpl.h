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

#ifndef UNITY_INTERNAL_SEARCHMETADATAIMPL_H
#define UNITY_INTERNAL_SEARCHMETADATAIMPL_H

#include <unity/scopes/internal/QueryMetadataImpl.h>
#include <unity/scopes/SearchMetadata.h>

#include <boost/optional.hpp>

namespace unity
{

namespace scopes
{

namespace internal
{

class SearchMetadataImpl : public QueryMetadataImpl
{
public:
    SearchMetadataImpl(std::string const& locale, std::string const& form_factor);
    SearchMetadataImpl(int cardinality, std::string const& locale, std::string const& form_factor);
    SearchMetadataImpl(VariantMap const& var);

    SearchMetadataImpl(SearchMetadataImpl const&) = default;
    SearchMetadataImpl(SearchMetadataImpl&&) = default;

    SearchMetadataImpl& operator=(SearchMetadataImpl const&) = default;
    SearchMetadataImpl& operator=(SearchMetadataImpl&&) = default;

    virtual ~SearchMetadataImpl() = default;

    void set_cardinality(int cardinality);
    int cardinality() const;

    void set_location(Location const& location);
    Location location() const;
    bool has_location() const;

    virtual VariantMap serialize() const override;

    static SearchMetadata create(VariantMap const& var);

protected:
    virtual std::string metadata_type() const override;
    virtual void serialize(VariantMap& var) const override;

    void check_cardinality(std::string const& func_name, int cardinality);

private:
    int cardinality_;
    boost::optional<Location> location_;
};

} // namespace internal

} // namespace scopes

} // namespace unity

#endif

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

#ifndef UNITY_INTERNAL_QUERYMETADATAIMPL_H
#define UNITY_INTERNAL_QUERYMETADATAIMPL_H

#include <unity/scopes/QueryMetadata.h>
#include <unity/scopes/Variant.h>
#include <string>

namespace unity
{

namespace scopes
{

namespace internal
{

class QueryMetadataImpl
{
public:
    QueryMetadataImpl(std::string const& locale, std::string const& form_factor);
    QueryMetadataImpl(VariantMap const& var);

    QueryMetadataImpl(QueryMetadataImpl const&) = default;
    QueryMetadataImpl(QueryMetadataImpl&&) = default;

    QueryMetadataImpl& operator=(QueryMetadataImpl const&) = default;
    QueryMetadataImpl& operator=(QueryMetadataImpl&&) = default;

    virtual ~QueryMetadataImpl() = default;

    std::string locale() const;
    std::string form_factor() const;

    void set_internet_connectivity(QueryMetadata::ConnectivityStatus connectivity_status);
    QueryMetadata::ConnectivityStatus internet_connectivity() const;

    bool contains_hint(std::string const& key) const;
    void set_hint(std::string const& key, Variant const& value);
    VariantMap hints() const;
    Variant& hint(std::string const& key);
    Variant const& hint(std::string const& key) const;

    virtual VariantMap serialize() const = 0;

protected:
    virtual void serialize(VariantMap& var) const = 0;
    virtual std::string metadata_type() const = 0;

private:
    std::string locale_;
    std::string form_factor_;
    VariantMap hints_;
    QueryMetadata::ConnectivityStatus internet_connectivity_;
};

} // namespace internal

} // namespace scopes

} // namespace unity

#endif

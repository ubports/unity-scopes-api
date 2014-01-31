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

#include <unity/scopes/GeoCoordinate.h>
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
    QueryMetadataImpl(std::string const& locale, std::string const& form_factor, GeoCoordinate const& location);
    virtual ~QueryMetadataImpl() = default;
    std::string locale() const;
    std::string form_factor() const;
    GeoCoordinate location() const;
    VariantMap serialize() const;

protected:
    virtual void serialize(VariantMap &var) const;
    virtual std::string metadata_type() const;

private:
    std::string locale_;
    std::string form_factor_;
    GeoCoordinate location_;
};

} // namespace internal

} // namespace scopes

} // namespace unity

#endif

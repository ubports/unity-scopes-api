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

#ifndef UNITY_INTERNAL_GEOCOORDINATEIMPL_H
#define UNITY_INTERNAL_GEOCOORDINATEIMPL_H

#include <unity/scopes/Variant.h>

namespace unity
{

namespace scopes
{

namespace internal
{

class GeoCoordinateImpl final
{
public:
    GeoCoordinateImpl(double lat, double lon);
    GeoCoordinateImpl(double lat, double lon, double alt);
    GeoCoordinateImpl(GeoCoordinateImpl const& other) = default;
    ~GeoCoordinateImpl();

    double latitude() const;
    double longitude() const;
    double altitude() const;
    bool has_altitude() const;
    VariantMap serialize() const;

private:
    double lat_;
    double lon_;
    double alt_;
    bool has_alt_;
};

} // namespace internal

} // namespace scopes

} // namespace unity

#endif

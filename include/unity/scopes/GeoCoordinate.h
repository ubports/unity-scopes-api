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

#ifndef UNITY_SCOPES_GEOCOORDINATE_H
#define UNITY_SCOPES_GEOCOORDINATE_H

#include <unity/SymbolExport.h>
#include <memory>

namespace unity
{

namespace scopes
{

namespace internal
{
class GeoCoordinateImpl;
}

/**
\brief
*/
class UNITY_API GeoCoordinate final
{
public:
    GeoCoordinate(double lat, double lon);
    GeoCoordinate(double lat, double lon, double alt);
    ~GeoCoordinate();

    GeoCoordinate(GeoCoordinate const& other);
    GeoCoordinate(GeoCoordinate&&);
    GeoCoordinate& operator=(GeoCoordinate const& other);
    GeoCoordinate& operator=(GeoCoordinate&&);

    double latitude() const;
    double longitude() const;
    double altitude() const;
    bool has_altitude() const;

private:
    std::unique_ptr<internal::GeoCoordinateImpl> p;
};

} // namespace scopes

} // namespace unity

#endif

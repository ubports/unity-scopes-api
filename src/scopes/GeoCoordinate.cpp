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

#include <unity/scopes/GeoCoordinate.h>
#include <unity/scopes/internal/GeoCoordinateImpl.h>

namespace unity
{

namespace scopes
{

GeoCoordinate::GeoCoordinate(double lat, double lon)
    : p(new internal::GeoCoordinateImpl(lat, lon))
{
}

GeoCoordinate::GeoCoordinate(double lat, double lon, double alt)
    : p(new internal::GeoCoordinateImpl(lat, lon, alt))
{
}

GeoCoordinate::GeoCoordinate(GeoCoordinate const& other)
    : p(new internal::GeoCoordinateImpl(*(other.p)))
{
}

GeoCoordinate& GeoCoordinate::operator=(GeoCoordinate const& other)
{
    if (this != &other)
    {
        p.reset(new internal::GeoCoordinateImpl(*(other.p)));
    }
    return *this;
}

GeoCoordinate::GeoCoordinate(GeoCoordinate&&) = default;
GeoCoordinate& GeoCoordinate::operator=(GeoCoordinate&&) = default;

GeoCoordinate::~GeoCoordinate()
{
}

double GeoCoordinate::latitude() const
{
    return p->latitude();
}

double GeoCoordinate::longitude() const
{
    return p->longitude();
}

double GeoCoordinate::altitude() const
{
    return p->altitude();
}

bool GeoCoordinate::has_altitude() const
{
    return p->has_altitude();
}

VariantMap GeoCoordinate::serialize() const
{
    return p->serialize();
}

} // namespace scopes

} // namespace unity

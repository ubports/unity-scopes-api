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

#include <unity/scopes/internal/GeoCoordinateImpl.h>
#include <unity/scopes/internal/Utils.h>

namespace unity
{

namespace scopes
{

namespace internal
{

GeoCoordinateImpl::GeoCoordinateImpl()
    : lat_(0.0f),
      lon_(0.0f),
      alt_(0.0f),
      has_alt_(false)
{
}

GeoCoordinateImpl::GeoCoordinateImpl(VariantMap const& var)
{
    static const std::string context("GeoCoordinateImpl()");
    auto it = find_or_throw(context, var, "lat");
    lat_ = it->second.get_double();
    it = find_or_throw(context, var, "lon");
    lon_ = it->second.get_double();
    it = find_or_throw(context, var, "alt");
    alt_ = it->second.get_double();
}

GeoCoordinateImpl::GeoCoordinateImpl(double lat, double lon)
    : lat_(lat),
      lon_(lon),
      alt_(0.0f),
      has_alt_(false)
{
}

GeoCoordinateImpl::GeoCoordinateImpl(double lat, double lon, double alt)
    : lat_(lat),
      lon_(lon),
      alt_(alt),
      has_alt_(true)
{
}

GeoCoordinateImpl::~GeoCoordinateImpl()
{
}

double GeoCoordinateImpl::latitude() const
{
    return lat_;
}

double GeoCoordinateImpl::longitude() const
{
    return lon_;
}

double GeoCoordinateImpl::altitude() const
{
    return alt_;
}

bool GeoCoordinateImpl::has_altitude() const
{
    return has_alt_;
}

VariantMap GeoCoordinateImpl::serialize() const
{
    VariantMap vm;
    vm["lat"] = Variant(lat_);
    vm["lon"] = Variant(lon_);
    vm["alt"] = Variant(alt_);
    vm["has_alt"] = Variant(has_alt_);
    return vm;
}

GeoCoordinate GeoCoordinateImpl::create(VariantMap const& var)
{
    return GeoCoordinate(new GeoCoordinateImpl(var));
}

} // namespace internal

} // namespace scopes

} // namespace unity

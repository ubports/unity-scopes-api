/*
 * Copyright (C) 2013 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the Lesser GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the Lesser GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Michi Henning <michi.henning@canonical.com>
 */

#include <unity/scopes/internal/LocationImpl.h>

#include <unity/scopes/ScopeExceptions.h>
#include <unity/UnityExceptions.h>

using namespace std;
using namespace unity;

namespace unity
{

namespace scopes
{

namespace internal
{

LocationImpl::LocationImpl(double latitude, double longitude)
    : latitude_(latitude)
    , longitude_(longitude)
{
}

LocationImpl::LocationImpl(const VariantMap& variant_map)
{
    deserialize(variant_map);
}

LocationImpl::LocationImpl(LocationImpl const& other)
    : altitude_(other.altitude_)
    , city_(other.city_)
    , country_code_(other.country_code_)
    , country_name_(other.country_name_)
    , horizontal_accuracy_(other.horizontal_accuracy_)
    , latitude_(other.latitude_)
    , longitude_(other.longitude_)
    , region_code_(other.region_code_)
    , region_name_(other.region_name_)
    , vertical_accuracy_(other.vertical_accuracy_)
    , zip_postal_code_(other.zip_postal_code_)
{
}

LocationImpl& LocationImpl::operator=(LocationImpl const& rhs)
{
    if (this != &rhs)
    {
        altitude_ = rhs.altitude_;
        area_code_ = rhs.area_code_;
        city_ = rhs.city_;
        country_code_ = rhs.country_code_;
        country_name_ = rhs.country_name_;
        horizontal_accuracy_ = rhs.horizontal_accuracy_;
        latitude_  = rhs.latitude_;
        longitude_  = rhs.longitude_;
        region_code_ = rhs.region_code_;
        region_name_ = rhs.region_name_;
        vertical_accuracy_ = rhs.vertical_accuracy_;
        zip_postal_code_ = rhs.zip_postal_code_;
    }
    return *this;
}

double LocationImpl::altitude() const
{
    if (altitude_)
    {
        return *altitude_;
    }
    throw NotFoundException("attribute not set", "altitude");
}

std::string LocationImpl::area_code() const
{
    if (area_code_)
    {
        return *area_code_;
    }
    throw NotFoundException("attribute not set", "area_code");
}

std::string LocationImpl::city() const
{
    if (city_)
    {
        return *city_;
    }
    throw NotFoundException("attribute not set", "city");
}

std::string LocationImpl::country_code() const
{
    if (country_code_)
    {
        return *country_code_;
    }
    throw NotFoundException("attribute not set", "country_code");
}

std::string LocationImpl::country_name() const
{
    if (country_name_)
    {
        return *country_name_;
    }
    throw NotFoundException("attribute not set", "country_name");
}

double LocationImpl::horizontal_accuracy() const
{
    if (horizontal_accuracy_)
    {
        return *horizontal_accuracy_;
    }
    throw NotFoundException("attribute not set", "horizontal_accuracy");
}

double LocationImpl::latitude() const
{
    if (latitude_)
    {
        return *latitude_;
    }
    throw NotFoundException("attribute not set", "latitude");
}

double LocationImpl::longitude() const
{
    if (longitude_)
    {
        return *longitude_;
    }
    throw NotFoundException("attribute not set", "longitude");
}

std::string LocationImpl::region_code() const
{
    if (region_code_)
    {
        return *region_code_;
    }
    throw NotFoundException("attribute not set", "region_code");
}

std::string LocationImpl::region_name() const
{
    if (region_name_)
    {
        return *region_name_;
    }
    throw NotFoundException("attribute not set", "region_name");
}

double LocationImpl::vertical_accuracy() const
{
    if (vertical_accuracy_)
    {
        return *vertical_accuracy_;
    }
    throw NotFoundException("attribute not set", "vertical_accuracy");
}

std::string LocationImpl::zip_postal_code() const
{
    if (zip_postal_code_)
    {
        return *zip_postal_code_;
    }
    throw NotFoundException("attribute not set", "zip_postal_code");
}

void LocationImpl::set_altitude(double altitude)
{
    altitude_ = altitude;
}

void LocationImpl::set_area_code(std::string const& area_code)
{
    area_code_ = area_code;
}

void LocationImpl::set_city(std::string const& city)
{
    city_ = city;
}

void LocationImpl::set_country_code(std::string const& country_code)
{
    country_code_ = country_code;
}

void LocationImpl::set_country_name(std::string const& country_name)
{
    country_name_ = country_name;
}

void LocationImpl::set_horizontal_accuracy(double horizontal_accuracy)
{
    horizontal_accuracy_ = horizontal_accuracy;
}

void LocationImpl::set_latitude(double latitude)
{
    latitude_ = latitude;
}

void LocationImpl::set_longitude(double longitude)
{
    longitude_ = longitude;
}

void LocationImpl::set_region_code(std::string const& region_code)
{
    region_code_ = region_code;
}

void LocationImpl::set_region_name(std::string const& region_name)
{
    region_name_ = region_name;
}

void LocationImpl::set_vertical_accuracy(double vertical_accuracy)
{
    vertical_accuracy_ = vertical_accuracy;
}

void LocationImpl::set_zip_postal_code(std::string const& zip_postal_code)
{
    zip_postal_code_ = zip_postal_code;
}

namespace
{

void throw_on_empty(std::string const& name, boost::optional<double> const& value)
{
    if (!value)
    {
        throw InvalidArgumentException("Location: required attribute '" + name + "' is empty");
    }
}

} // namespace

VariantMap LocationImpl::serialize() const
{
    throw_on_empty("latitude", latitude_);
    throw_on_empty("longitude", longitude_);

    VariantMap var;
    var["latitude"] = *latitude_;
    var["longitude"] = *longitude_;

    // Optional fields
    if (altitude_)
    {
        var["altitude"] = *altitude_;
    }
    if (area_code_)
    {
        var["area_code"] = *area_code_;
    }
    if (city_)
    {
        var["city"] = *city_;
    }
    if (country_code_)
    {
        var["country_code"] = *country_code_;
    }
    if (country_name_)
    {
        var["country_name"] = *country_name_;
    }
    if (horizontal_accuracy_)
    {
        var["horizontal_accuracy"] = *horizontal_accuracy_;
    }
    if (region_code_)
    {
        var["region_code"] = *region_code_;
    }
    if (region_name_)
    {
        var["region_name"] = *region_name_;
    }
    if (vertical_accuracy_)
    {
        var["vertical_accuracy"] = *vertical_accuracy_;
    }
    if (zip_postal_code_)
    {
        var["zip_postal_code"] = *zip_postal_code_;
    }

    return var;
}

namespace
{

VariantMap::const_iterator find_or_throw(VariantMap const& var, std::string const& name)
{
    auto it = var.find(name);
    if (it == var.end())
    {
        throw InvalidArgumentException("Location::deserialize(): required attribute '" + name + "' is missing");
    }
    return it;
}

} // namespace

void LocationImpl::deserialize(VariantMap const& var)
{
    auto it = find_or_throw(var, "latitude");
    latitude_ = it->second.get_double();

    it = find_or_throw(var, "longitude");
    longitude_ = it->second.get_double();

    // Optional fields

    it = var.find("altitude");
    if (it != var.end())
    {
        altitude_ = it->second.get_double();
    }

    it = var.find("area_code");
    if (it != var.end())
    {
        area_code_ = it->second.get_string();
    }

    it = var.find("city");
    if (it != var.end())
    {
        city_ = it->second.get_string();
    }

    it = var.find("country_code");
    if (it != var.end())
    {
        country_code_ = it->second.get_string();
    }

    it = var.find("country_name");
    if (it != var.end())
    {
        country_name_ = it->second.get_string();
    }

    it = var.find("horizontal_accuracy");
    if (it != var.end())
    {
        horizontal_accuracy_ = it->second.get_double();
    }

    it = var.find("region_code");
    if (it != var.end())
    {
        region_code_ = it->second.get_string();
    }

    it = var.find("region_name");
    if (it != var.end())
    {
        region_name_ = it->second.get_string();
    }

    it = var.find("vertical_accuracy");
    if (it != var.end())
    {
        vertical_accuracy_ = it->second.get_double();
    }

    it = var.find("zip_postal_code");
    if (it != var.end())
    {
        zip_postal_code_ = it->second.get_string();
    }
}

} // namespace internal

} // namespace scopes

} // namespace unity

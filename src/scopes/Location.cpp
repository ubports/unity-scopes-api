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
 * Authored by: Pete Woods <pete.woods@canonical.com>
 */

#include <unity/scopes/Location.h>
#include <unity/scopes/internal/LocationImpl.h>

#include <cassert>

using namespace std;

namespace unity
{

namespace scopes
{

//! @cond

Location::Location(double latitude, double longitude) :
        p(new internal::LocationImpl(latitude, longitude))
{
}

Location::Location(VariantMap const& variant) :
    p(new internal::LocationImpl(variant))
{
}

Location::Location(Location const& other) :
    p(new internal::LocationImpl(*(other.p)))
{
}

Location::Location(Location&&) = default;

Location::~Location()
{
}

Location& Location::operator=(Location const& other)
{
    if (this != &other)
    {
        p.reset(new internal::LocationImpl(*(other.p)));
    }
    return *this;
}

Location& Location::operator=(Location&&) = default;

double Location::altitude() const
{
    return p->altitude();
}

bool Location::has_altitude() const
{
    return p->has_altitude();
}

string Location::area_code() const
{
    return p->area_code();
}

bool Location::has_area_code() const
{
    return p->has_area_code();
}

string Location::city() const
{
    return p->city();
}

bool Location::has_city() const
{
    return p->has_city();
}

string Location::country_code() const
{
    return p->country_code();
}

bool Location::has_country_code() const
{
    return p->has_country_code();
}

string Location::country_name() const
{
    return p->country_name();
}

bool Location::has_country_name() const
{
    return p->has_country_name();
}

double Location::horizontal_accuracy() const
{
    return p->horizontal_accuracy();
}

bool Location::has_horizontal_accuracy() const
{
    return p->has_horizontal_accuracy();
}

double Location::latitude() const
{
    return p->latitude();
}

double Location::longitude() const
{
    return p->longitude();
}

VariantMap Location::serialize() const
{
    return p->serialize();
}

string Location::region_code() const
{
    return p->region_code();
}

bool Location::has_region_code() const
{
    return p->has_region_code();
}

string Location::region_name() const
{
    return p->region_name();
}

bool Location::has_region_name() const
{
    return p->has_region_name();
}

double Location::vertical_accuracy() const
{
    return p->vertical_accuracy();
}

bool Location::has_vertical_accuracy() const
{
    return p->has_vertical_accuracy();
}

string Location::zip_postal_code() const
{
    return p->zip_postal_code();
}

bool Location::has_zip_postal_code() const
{
    return p->has_zip_postal_code();
}

void Location::set_altitude(double altitude)
{
    p->set_altitude(altitude);
}

void Location::set_area_code(string const& area_code)
{
    p->set_area_code(area_code);
}

void Location::set_city(string const& city)
{
    p->set_city(city);
}

void Location::set_country_code(string const& country_code)
{
    p->set_country_code(country_code);
}

void Location::set_country_name(string const& country_name)
{
    p->set_country_name(country_name);
}

void Location::set_horizontal_accuracy(double horizontal_accuracy)
{
    p->set_horizontal_accuracy(horizontal_accuracy);
}

void Location::set_latitude(double latitude)
{
    p->set_latitude(latitude);
}

void Location::set_longitude(double longitude)
{
    p->set_longitude(longitude);
}

void Location::set_region_code(string const& region_code)
{
    p->set_region_code(region_code);
}

void Location::set_region_name(string const& region_name)
{
    p->set_region_name(region_name);
}

void Location::set_vertical_accuracy(double vertical_accuracy)
{
    p->set_vertical_accuracy(vertical_accuracy);
}

void Location::set_zip_postal_code(string const& zip_postal_code)
{
    p->set_zip_postal_code(zip_postal_code);
}

//! @endcond

} // namespace scopes

} // namespace unity

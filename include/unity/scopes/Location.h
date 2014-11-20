/*
 * Copyright (C) 2014 Canonical Ltd
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
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Pete Woods <pete.wods@canonical.com>
 */

#pragma once

#include <unity/scopes/Variant.h>
#include <unity/util/DefinesPtrs.h>

namespace unity
{

namespace scopes
{

namespace internal
{

class LocationImpl;
} // namespace internal

/**
\brief Holds location attributes such as latitude, longitude, etc.
*/
class Location final
{
public:
    /// @cond
    UNITY_DEFINES_PTRS(Location);
    /// @endcond

    /**
     \brief Construct a new Location with the specified latitude and longitude
     \param latitude Latitude
     \param longitude Longitude
     */
    Location(double latitude, double longitude);

    /**
     \brief Construct a Location from a the given serialized VariantMap
     */
    Location(VariantMap const& variant);

    /// @cond
    ~Location();
    /// @endcond

    /**@name Copy and assignment
    Copy and assignment operators (move and non-move versions) have the usual value semantics.
    */
    //{@
    Location(Location const& other);
    Location(Location&&);

    Location& operator=(Location const& other);
    Location& operator=(Location&&);
    //@}

    /**
    \brief Get the altitude.
    \return The altitude.
    \throws unity::scopes::NotFoundException if altitude is not set.
    */
    double altitude() const;            // optional

    /**
    \brief Is there an altitude property.
    \return True if there is an altitude property.
    */
    bool has_altitude() const;

    /**
    \brief Get the area code.
    \return The area code.
    \throws unity::scopes::NotFoundException if area code is not set.
    */
    std::string area_code() const;            // optional

    /**
    \brief Is there an area code property.
    \return True if there is an area code property.
    */
    bool has_area_code() const;

    /**
    \brief Get the city name.
    \return The city name.
    \throws unity::scopes::NotFoundException if city is not set.
    */
    std::string city() const;            // optional

    /**
    \brief Is there a city property.
    \return True if there is a city property.
    */
    bool has_city() const;

    /**
    \brief Get the country code.
    \return The country code.
    \throws unity::scopes::NotFoundException if country code is not set.
    */
    std::string country_code() const;            // optional

    /**
    \brief Is there a country code property.
    \return True if there is a country code property.
    */
    bool has_country_code() const;

    /**
    \brief Get the country name.
    \return The country name.
    \throws unity::scopes::NotFoundException if country name is not set.
    */
    std::string country_name() const;            // optional

    /**
    \brief Is there a country name property.
    \return True if there is a country name property.
    */
    bool has_country_name() const;

    /**
    \brief Get the horizontal accuracy.
    \return The horizontal accuracy.
    \throws unity::scopes::NotFoundException if horizontal accuracy is not set.
    */
    double horizontal_accuracy() const;            // optional

    /**
    \brief Is there a horizontal accuracy property.
    \return True if there is a horizontal accuracy property.
    */
    bool has_horizontal_accuracy() const;

    /**
    \brief Get the latitude.
    \return The latitude.
    */
    double latitude() const;

    /**
    \brief Get the longitude.
    \return The longitude.
    */
    double longitude() const;

    /**
    \brief Return a dictionary of all location attributes.
    \return Dictionary of all location attributes.
    */
    VariantMap serialize() const;

    /**
    \brief Get the region code.
    \return The region code.
    \throws unity::scopes::NotFoundException if region code is not set.
    */
    std::string region_code() const;            // optional

    /**
    \brief Is there a region code property.
    \return True if there is a region code property.
    */
    bool has_region_code() const;

    /**
    \brief Get the region name.
    \return The region name.
    \throws unity::scopes::NotFoundException if region name is not set.
    */
    std::string region_name() const;            // optional

    /**
    \brief Is there a region name property.
    \return True if there is a region name property.
    */
    bool has_region_name() const;

    /**
    \brief Get the vertical accuracy.
    \return The vertical accuracy.
    \throws unity::scopes::NotFoundException if vertical accuracy is not set.
    */
    double vertical_accuracy() const;            // optional

    /**
    \brief Is there a vertical accuracy property.
    \return True if there is a vertical accuracy property.
    */
    bool has_vertical_accuracy() const;

    /**
    \brief Get the zip or postal code.
    \return The zip or postal code.
    \throws unity::scopes::NotFoundException if postal code is not set.
    */
    std::string zip_postal_code() const;            // optional

    /**
    \brief Is there a zip / postal code property.
    \return True if there is a zip / postal code property.
    */
    bool has_zip_postal_code() const;

    /**
    \brief Set the altitude.
    \param altitude In meters.
    */
    void set_altitude(double altitude);

    /**
    \brief Set the area code.
    \param area_code FIPS10-4 area code.
    */
    void set_area_code(std::string const& area_code);

    /**
    \brief Set the city name.
    \param city Name of the city.
    */
    void set_city(std::string const& city);

    /**
    \brief Set the country code.
    \param country_code FIPS10-4 country code.
    */
    void set_country_code(std::string const& country_code);

    /**
    \brief Set the country name.
    \param country_name Human readable country name.
    */
    void set_country_name(std::string const& country_name);

    /**
    \brief Set the horizontal accuracy.
    \param horizontal_accuracy Horizontal accouracy.
    */
    void set_horizontal_accuracy(double horizontal_accuracy);

    /**
    \brief Set the latitude.
    \param latitude Latitude.
    */
    void set_latitude(double latitude);

    /**
    \brief Set the longitude.
    \param longitude Latitude.
    */
    void set_longitude(double longitude);

    /**
    \brief Set the region code.
    \param region_code FIPS10-4 region code.
    */
    void set_region_code(std::string const& region_code);

    /**
    \brief Set the region name.
    \param region_name Human readable region name.
    */
    void set_region_name(std::string const& region_name);

    /**
    \brief Set the vertical_accuracy.
    \param vertical_accuracy Vertical accuracy in meters.
    */
    void set_vertical_accuracy(double vertical_accuracy);

    /**
    \brief Set the zip/postal code.
    \param zip_postal_code Either a zip or postal code.
    */
    void set_zip_postal_code(std::string const& zip_postal_code);

private:
    std::unique_ptr<internal::LocationImpl> p;

    friend class internal::LocationImpl;
};

} // namespace scopes

} // namespace unity

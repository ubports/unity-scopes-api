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
 * You should have received a copy of the Lesser GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Pete Woods <pete.woods@canonical.com>
 */

#ifndef UNITY_SCOPES_INTERNAL_LOCATIONIMPL_H
#define UNITY_SCOPES_INTERNAL_LOCATIONIMPL_H

#include <unity/scopes/Location.h>

#include <unordered_set>

#include <boost/optional.hpp>

namespace unity
{

namespace scopes
{

namespace internal
{

class LocationImpl
{
public:
    LocationImpl(double latitude, double longitude);
    LocationImpl(VariantMap const& variant_map);
    LocationImpl(LocationImpl const&);
    LocationImpl(LocationImpl&&) = default;
    LocationImpl& operator=(LocationImpl const&);
    LocationImpl& operator=(LocationImpl&&) = default;

    double altitude() const;                                     // optional
    std::string area_code() const;                               // optional
    std::string city() const;                                    // optional
    std::string country_code() const;                            // optional
    std::string country_name() const;                            // optional
    double horizontal_accuracy() const;                          // optional
    double latitude() const;                                     // mandatory
    double longitude() const;                                    // mandatory
    std::string region_code() const;                             // optional
    std::string region_name() const;                             // optional
    double vertical_accuracy() const;                            // optional
    std::string zip_postal_code() const;                         // optional

    bool has_altitude() const;
    bool has_area_code() const;
    bool has_city() const;
    bool has_country_code() const;
    bool has_country_name() const;
    bool has_horizontal_accuracy() const;
    bool has_region_code() const;
    bool has_region_name() const;
    bool has_vertical_accuracy() const;
    bool has_zip_postal_code() const;

    void set_altitude(double altitude);
    void set_area_code(std::string const& area_code);
    void set_city(std::string const& city);
    void set_country_code(std::string const& country_code);
    void set_country_name(std::string const& country_name);
    void set_horizontal_accuracy(double horizontal_accuracy);
    void set_latitude(double latitude);
    void set_longitude(double longitude);
    void set_region_code(std::string const& region_code);
    void set_region_name(std::string const& region_name);
    void set_vertical_accuracy(double vertical_accuracy);
    void set_zip_postal_code(std::string const& zip_postal_code);

    VariantMap serialize() const;
    void deserialize(VariantMap const& var);

private:
    boost::optional<double> altitude_;
    boost::optional<std::string> area_code_;
    boost::optional<std::string> city_;
    boost::optional<std::string> country_code_;
    boost::optional<std::string> country_name_;
    boost::optional<double> horizontal_accuracy_;
    boost::optional<double> latitude_;
    boost::optional<double> longitude_;
    boost::optional<std::string> region_code_;
    boost::optional<std::string> region_name_;
    boost::optional<double> vertical_accuracy_;
    boost::optional<std::string> zip_postal_code_;
};

} // namespace internal

} // namespace scopes

} // namespace unity


#endif

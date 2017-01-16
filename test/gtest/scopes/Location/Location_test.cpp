/*
 * Copyright (C) 2013 Canonical Ltd
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

#include <unity/scopes/ScopeExceptions.h>
#include <unity/scopes/Location.h>
#include <unity/UnityExceptions.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <gtest/gtest.h>
#pragma GCC diagnostic pop

using namespace unity;
using namespace unity::scopes;
using namespace unity::scopes::internal;

namespace
{

class LocationTest: public testing::Test {
};

TEST_F(LocationTest, basic)
{
    Location loc(2.0, 4.0);
    EXPECT_DOUBLE_EQ(2.0, loc.latitude());
    EXPECT_DOUBLE_EQ(4.0, loc.longitude());
}

TEST_F(LocationTest, optionals_throw_when_unset)
{
    Location loc(1.0, 2.0);

    EXPECT_FALSE(loc.has_altitude());
    EXPECT_FALSE(loc.has_area_code());
    EXPECT_FALSE(loc.has_city());
    EXPECT_FALSE(loc.has_country_code());
    EXPECT_FALSE(loc.has_country_name());
    EXPECT_FALSE(loc.has_horizontal_accuracy());
    EXPECT_FALSE(loc.has_region_code());
    EXPECT_FALSE(loc.has_region_name());
    EXPECT_FALSE(loc.has_vertical_accuracy());
    EXPECT_FALSE(loc.has_zip_postal_code());

    EXPECT_THROW(loc.altitude(), NotFoundException);
    EXPECT_THROW(loc.area_code(), NotFoundException);
    EXPECT_THROW(loc.city(), NotFoundException);
    EXPECT_THROW(loc.country_code(), NotFoundException);
    EXPECT_THROW(loc.country_name(), NotFoundException);
    EXPECT_THROW(loc.horizontal_accuracy(), NotFoundException);
    EXPECT_THROW(loc.region_code(), NotFoundException);
    EXPECT_THROW(loc.region_name(), NotFoundException);
    EXPECT_THROW(loc.vertical_accuracy(), NotFoundException);
    EXPECT_THROW(loc.zip_postal_code(), NotFoundException);
}

TEST_F(LocationTest, setters_getters)
{
    Location loc(1.0, 2.0);

    EXPECT_FALSE(loc.has_altitude());
    EXPECT_FALSE(loc.has_area_code());
    EXPECT_FALSE(loc.has_city());
    EXPECT_FALSE(loc.has_country_code());
    EXPECT_FALSE(loc.has_country_name());
    EXPECT_FALSE(loc.has_horizontal_accuracy());
    EXPECT_FALSE(loc.has_region_code());
    EXPECT_FALSE(loc.has_region_name());
    EXPECT_FALSE(loc.has_vertical_accuracy());
    EXPECT_FALSE(loc.has_zip_postal_code());

    loc.set_altitude(20.0);
    EXPECT_TRUE(loc.has_altitude());
    EXPECT_DOUBLE_EQ(20.0, loc.altitude());

    loc.set_area_code("area code");
    EXPECT_TRUE(loc.has_area_code());
    EXPECT_EQ("area code", loc.area_code());

    loc.set_city("city");
    EXPECT_TRUE(loc.has_city());
    EXPECT_EQ("city", loc.city());

    loc.set_country_code("country code");
    EXPECT_TRUE(loc.has_country_code());
    EXPECT_EQ("country code", loc.country_code());

    loc.set_country_name("country name");
    EXPECT_TRUE(loc.has_country_name());
    EXPECT_EQ("country name", loc.country_name());

    loc.set_horizontal_accuracy(25.0);
    EXPECT_TRUE(loc.has_horizontal_accuracy());
    EXPECT_DOUBLE_EQ(25.0, loc.horizontal_accuracy());

    loc.set_latitude(5.0);
    EXPECT_DOUBLE_EQ(5.0, loc.latitude());

    loc.set_longitude(6.0);
    EXPECT_DOUBLE_EQ(6.0, loc.longitude());

    loc.set_region_code("region code");
    EXPECT_TRUE(loc.has_region_code());
    EXPECT_EQ("region code", loc.region_code());

    loc.set_region_name("region name");
    EXPECT_TRUE(loc.has_region_name());
    EXPECT_EQ("region name", loc.region_name());

    loc.set_vertical_accuracy(4.0);
    EXPECT_TRUE(loc.has_vertical_accuracy());
    EXPECT_DOUBLE_EQ(4.0, loc.vertical_accuracy());

    loc.set_zip_postal_code("zip postal code");
    EXPECT_TRUE(loc.has_zip_postal_code());
    EXPECT_EQ("zip postal code", loc.zip_postal_code());
}

TEST_F(LocationTest, serialize_mandatory_only)
{
    Location loc(1.0, 2.0);

    auto var = loc.serialize();
    ASSERT_EQ(2u, var.size());
    EXPECT_DOUBLE_EQ(1.0, var["latitude"].get_double());
    EXPECT_DOUBLE_EQ(2.0, var["longitude"].get_double());

    Location loc2(var);
    EXPECT_DOUBLE_EQ(1.0, loc2.latitude());
    EXPECT_DOUBLE_EQ(2.0, loc2.longitude());
}

TEST_F(LocationTest, serialize)
{
    Location loc(5.0, 6.0);
    loc.set_altitude(20.0);
    loc.set_area_code("area code");
    loc.set_city("city");
    loc.set_country_code("country code");
    loc.set_country_name("country name");
    loc.set_horizontal_accuracy(25.0);
    loc.set_region_code("region code");
    loc.set_region_name("region name");
    loc.set_vertical_accuracy(4.0);
    loc.set_zip_postal_code("zip postal code");

    auto var = loc.serialize();
    ASSERT_EQ(12u, var.size());
    EXPECT_DOUBLE_EQ(5.0, var["latitude"].get_double());
    EXPECT_DOUBLE_EQ(6.0, var["longitude"].get_double());
    EXPECT_DOUBLE_EQ(20.0, var["altitude"].get_double());
    EXPECT_EQ("area code", var["area_code"].get_string());
    EXPECT_EQ("city", var["city"].get_string());
    EXPECT_EQ("country code", var["country_code"].get_string());
    EXPECT_EQ("country name", var["country_name"].get_string());
    EXPECT_DOUBLE_EQ(25.0, var["horizontal_accuracy"].get_double());
    EXPECT_EQ("region code", var["region_code"].get_string());
    EXPECT_EQ("region name", var["region_name"].get_string());
    EXPECT_DOUBLE_EQ(4.0, var["vertical_accuracy"].get_double());
    EXPECT_EQ("zip postal code", var["zip_postal_code"].get_string());

    Location loc2(var);
    EXPECT_DOUBLE_EQ(5.0, loc2.latitude());
    EXPECT_DOUBLE_EQ(6.0, loc2.longitude());
    EXPECT_DOUBLE_EQ(20.0, loc2.altitude());
    EXPECT_EQ("area code", loc2.area_code());
    EXPECT_EQ("city", loc2.city());
    EXPECT_EQ("country code", loc2.country_code());
    EXPECT_EQ("country name", loc2.country_name());
    EXPECT_DOUBLE_EQ(25.0, loc2.horizontal_accuracy());
    EXPECT_EQ("region code", loc2.region_code());
    EXPECT_EQ("region name", loc2.region_name());
    EXPECT_DOUBLE_EQ(4.0, loc2.vertical_accuracy());
    EXPECT_EQ("zip postal code", loc2.zip_postal_code());
}

TEST_F(LocationTest, test_assignment)
{
    Location loc(5.0, 6.0);
    loc.set_altitude(20.0);
    loc.set_area_code("area code");
    loc.set_city("city");
    loc.set_country_code("country code");
    loc.set_country_name("country name");
    loc.set_horizontal_accuracy(25.0);
    loc.set_region_code("region code");
    loc.set_region_name("region name");
    loc.set_vertical_accuracy(4.0);
    loc.set_zip_postal_code("zip postal code");

    Location loc2 = loc;
    EXPECT_DOUBLE_EQ(5.0, loc2.latitude());
    EXPECT_DOUBLE_EQ(6.0, loc2.longitude());
    EXPECT_DOUBLE_EQ(20.0, loc2.altitude());
    EXPECT_EQ("area code", loc2.area_code());
    EXPECT_EQ("city", loc2.city());
    EXPECT_EQ("country code", loc2.country_code());
    EXPECT_EQ("country name", loc2.country_name());
    EXPECT_DOUBLE_EQ(25.0, loc2.horizontal_accuracy());
    EXPECT_EQ("region code", loc2.region_code());
    EXPECT_EQ("region name", loc2.region_name());
    EXPECT_DOUBLE_EQ(4.0, loc2.vertical_accuracy());
    EXPECT_EQ("zip postal code", loc2.zip_postal_code());

    Location loc3(5.0, 6.0);
    loc3 = loc;
    EXPECT_DOUBLE_EQ(5.0, loc3.latitude());
    EXPECT_DOUBLE_EQ(6.0, loc3.longitude());
    EXPECT_DOUBLE_EQ(20.0, loc3.altitude());
    EXPECT_EQ("area code", loc3.area_code());
    EXPECT_EQ("city", loc3.city());
    EXPECT_EQ("country code", loc3.country_code());
    EXPECT_EQ("country name", loc3.country_name());
    EXPECT_DOUBLE_EQ(25.0, loc3.horizontal_accuracy());
    EXPECT_EQ("region code", loc3.region_code());
    EXPECT_EQ("region name", loc3.region_name());
    EXPECT_DOUBLE_EQ(4.0, loc3.vertical_accuracy());
    EXPECT_EQ("zip postal code", loc3.zip_postal_code());
}

}

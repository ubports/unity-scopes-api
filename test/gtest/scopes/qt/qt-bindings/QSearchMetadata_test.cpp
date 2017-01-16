/*
 * Copyright (C) 2015 Canonical Ltd
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
 * Authored by: Xavi Garcia <xavi.garcia.mena@canonical.com>
 */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <gtest/gtest.h>
#pragma GCC diagnostic pop

#include <unity/scopes/qt/QSearchMetadata.h>

using namespace unity::scopes::qt;

TEST(QActionMetadata, bindings)
{
    QSearchMetadata* metadata = new QSearchMetadata("en", "phone");

    QVariant data(1999);

    // hints tests
    EXPECT_EQ(metadata->contains_hint("test_key"), false);
    metadata->set_hint(QString("test_key"), QVariant("test_value"));
    EXPECT_EQ(metadata->contains_hint("test_key"), true);
    QVariant stored_data = (*metadata)["test_key"];
    EXPECT_EQ(stored_data.toString(), "test_value");

    (*metadata)["test_key2"] = QVariant("test_value2");
    EXPECT_TRUE((*metadata)["test_key2"].toString() == "test_value2");

    (*metadata)["test_key3"] = QVariant("test_value3");
    QVariantMap map_hints = metadata->hints();
    EXPECT_EQ(map_hints.size(), 3);

    EXPECT_EQ(map_hints["test_key"].toString(), "test_value");
    EXPECT_EQ(map_hints["test_key2"].toString(), "test_value2");
    EXPECT_EQ(map_hints["test_key3"].toString(), "test_value3");
    QVariant null_variant = map_hints["test_key4"];
    EXPECT_EQ(null_variant.isNull(), true);

    (*metadata)["test_key2"] = QVariant("test_value22");

    QSearchMetadata metadata2 = *metadata;
    delete metadata;

    metadata2["test_key3"] = QVariant("test_value33");
    EXPECT_TRUE(metadata2["test_key3"].toString() == "test_value33");
    EXPECT_TRUE(metadata2["test_key2"].toString() == "test_value22");

    map_hints = metadata2.hints();
    EXPECT_EQ(map_hints.size(), 3);

    EXPECT_EQ(map_hints["test_key"].toString(), "test_value");
    EXPECT_EQ(map_hints["test_key2"].toString(), "test_value22");
    EXPECT_EQ(map_hints["test_key3"].toString(), "test_value33");

    metadata2.set_hint(QString("test_key"), QVariant("test_value_new"));
    EXPECT_TRUE(metadata2["test_key"].toString() == "test_value_new");
}

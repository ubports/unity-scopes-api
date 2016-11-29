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
 * Authored by: Michi Henning <michi.henning@canonical.com>
 */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <gtest/gtest.h>
#pragma GCC diagnostic pop

#include <unity/scopes/qt/internal/QUtils.h>

#include <unity/UnityExceptions.h>

using namespace unity::scopes::qt::internal;
using namespace unity::scopes;

TEST(QUtils, variant_to_qvariant)
{
    {
        Variant v;
        QVariant qv = variant_to_qvariant(v);
        EXPECT_FALSE(qv.isValid());
    }

    {
        Variant v(42);
        QVariant qv = variant_to_qvariant(v);
        EXPECT_EQ(42, qv.toInt());
    }

    {
        Variant v(true);
        QVariant qv = variant_to_qvariant(v);
        EXPECT_TRUE(qv.toBool());
    }

    {
        Variant v("Hello");
        QVariant qv = variant_to_qvariant(v);
        EXPECT_EQ("Hello", qv.toString());
    }

    {
        Variant v(3.14);
        QVariant qv = variant_to_qvariant(v);
        EXPECT_EQ(3.14, qv.toDouble());
    }

    {
        VariantMap vm;
        vm["key"] = "val";
        Variant v(vm);
        QVariant qv = variant_to_qvariant(v);
        EXPECT_EQ("val", qv.toMap().value("key").toString());
    }

    {
        VariantArray va;
        va.push_back(Variant(42));
        va.push_back(Variant("Hello"));
        Variant v(va);
        QVariant qv = variant_to_qvariant(v);
        ASSERT_EQ(2, qv.toList().size());
        EXPECT_EQ(42, qv.toList().at(0).toInt());
        EXPECT_EQ("Hello", qv.toList().at(1).toString());
    }
}

TEST(QUtils, qvariant_to_variant)
{
    {
        QVariant qv;
        Variant v = qvariant_to_variant(qv);
        EXPECT_TRUE(v.is_null());
    }

    {
        QVariant qv(true);
        Variant v = qvariant_to_variant(qv);
        EXPECT_TRUE(v.get_bool());
    }

    {
        QVariant qv(42);
        Variant v = qvariant_to_variant(qv);
        EXPECT_EQ(42, v.get_int());
    }

    {
        QVariant qv(3.14);
        Variant v = qvariant_to_variant(qv);
        EXPECT_EQ(3.14, v.get_double());
    }

    {
        QVariant qv("sss");
        Variant v = qvariant_to_variant(qv);
        EXPECT_EQ("sss", v.get_string());
    }

    {
        QVariantMap qvm;
        qvm["key"] = QVariant("val");
        Variant v = qvariant_to_variant(QVariant(qvm));
        EXPECT_EQ("val", v.get_dict()["key"].get_string());
    }

    {
        QVariantList qvl;
        qvl.push_back(QVariant(3.14));
        Variant v = qvariant_to_variant(QVariant(qvl));
        EXPECT_EQ(3.14, v.get_array()[0].get_double());
    }

    {
        QVariant qv(QChar('a'));
        try
        {
            Variant v = qvariant_to_variant(qv);
            FAIL();
        }
        catch (unity::InvalidArgumentException const& e)
        {
            EXPECT_STREQ("unity::InvalidArgumentException: qvariant_to_variant(): invalid source type: QChar",
                         e.what());
        }
        catch (std::exception const& e)
        {
            FAIL() << e.what();
        }
    }
}

TEST(QUtils, variantmap_to_qvariantmap)
{
    VariantMap vm;
    vm["int"] = 42;
    vm["string"] = "Hello";

    QVariantMap qvm = variantmap_to_qvariantmap(vm);
    ASSERT_EQ(2, qvm.count());
    EXPECT_EQ(42, qvm.value("int").toInt());
    EXPECT_EQ("Hello", qvm.value("string").toString());
}

TEST(QUtils, qvariantmap_to_variantmap)
{
    QVariantMap qvm;
    qvm["int"] = 42;
    qvm["string"] = "Hello";

    VariantMap vm = qvariantmap_to_variantmap(qvm);
    ASSERT_EQ(2u, vm.size());
    EXPECT_EQ(42, vm["int"].get_int());
    EXPECT_EQ("Hello", vm["string"].get_string());
}

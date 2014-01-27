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
 * Authored by: Pawel Stolowski <pawel.stolowski@canonical.com>
 */

#include <unity/scopes/PreviewWidget.h>
#include <unity/scopes/internal/PreviewWidgetImpl.h>
#include <unity/UnityExceptions.h>
#include <unity/scopes/Variant.h>
#include <unity/scopes/internal/JsonCppNode.h>
#include <gtest/gtest.h>

using namespace unity::scopes;
using namespace unity::scopes::internal;

TEST(PreviewWidget, basic)
{
    PreviewWidget w("i1", "image");
    w.add_attribute("foo", Variant(10));
    w.add_component("boo", "bar");

    EXPECT_EQ("i1", w.id());
    EXPECT_EQ("image", w.widget_type());
    EXPECT_EQ(1, w.attributes().size());
    EXPECT_EQ(10, w.attributes()["foo"].get_int());
    EXPECT_EQ(1, w.components().size());
    EXPECT_EQ("bar", w.components()["boo"].get_string());

    internal::JsonCppNode node(w.data());
    EXPECT_EQ("i1", node.get_node("id")->as_string());
    EXPECT_EQ("image", node.get_node("type")->as_string());
    EXPECT_EQ(10, node.get_node("foo")->as_int());
    EXPECT_EQ("bar", node.get_node("components")->get_node("boo")->as_string());
}

TEST(PreviewWidget, exceptions)
{
    {
        PreviewWidget w("a", "image");
        EXPECT_THROW(w.set_id(""), unity::InvalidArgumentException);
        EXPECT_THROW(w.set_widget_type(""), unity::InvalidArgumentException);
    }
    {
        PreviewWidget w("a", "image");
        EXPECT_THROW(w.add_attribute("id", Variant("x")), unity::InvalidArgumentException);
        EXPECT_THROW(w.add_attribute("type", Variant("x")), unity::InvalidArgumentException);
        EXPECT_THROW(w.add_component("id", "x"), unity::InvalidArgumentException);
        EXPECT_THROW(w.add_component("type", "x"), unity::InvalidArgumentException);
    }
}

TEST(PreviewWidget, serialize)
{
    {
        PreviewWidget w("i1", "image");
        w.add_attribute("foo", Variant(10));
        w.add_component("boo", "bar");

        auto var = w.serialize();
        EXPECT_EQ("i1", var["id"].get_string());
        EXPECT_EQ("image", var["type"].get_string());
        EXPECT_EQ(10, var["attributes"].get_dict()["foo"].get_int());
        EXPECT_EQ("bar", var["components"].get_dict()["boo"].get_string());
    }
}

TEST(PreviewWidget, deserialize)
{
    {
        VariantMap attrs;
        attrs["foo"] = "bar";
        VariantMap comps;
        comps["bee"] = "cee";
        VariantMap outer;
        outer["id"] = "i1";
        outer["type"] = "image";
        outer["attributes"] = Variant(attrs);
        outer["components"] = Variant(comps);

        auto w = internal::PreviewWidgetImpl::create(outer);
        EXPECT_EQ("i1", w.id());
        EXPECT_EQ("image", w.widget_type());
        EXPECT_EQ(1, w.attributes().size());
        EXPECT_EQ("bar", w.attributes()["foo"].get_string());
        EXPECT_EQ(1, w.components().size());
        EXPECT_EQ("cee", w.components()["bee"].get_string());
    }
}

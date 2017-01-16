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

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <gtest/gtest.h>
#pragma GCC diagnostic pop

using namespace unity::scopes;
using namespace unity::scopes::internal;

TEST(PreviewWidget, basic)
{
    {
        PreviewWidget w("i1", "image");
        w.add_attribute_value("foo", Variant(10));
        w.add_attribute_mapping("boo", "bar");

        EXPECT_EQ("i1", w.id());
        EXPECT_EQ("image", w.widget_type());
        EXPECT_EQ(1u, w.attribute_values().size());
        EXPECT_EQ(10, w.attribute_values()["foo"].get_int());
        EXPECT_EQ(1u, w.attribute_mappings().size());
        EXPECT_EQ("bar", w.attribute_mappings()["boo"]);
    }
}

TEST(PreviewWidget, expandable_widget)
{
    {
        PreviewWidget w("w1", "expandable");
        w.add_attribute_value("title", Variant("foo"));

        {
            PreviewWidget subwidget1("w2", "image");
            subwidget1.add_attribute_mapping("source", "src");

            PreviewWidget subwidget2("w3", "image");
            subwidget2.add_attribute_value("source", Variant("baz"));

            w.add_widget(subwidget1);
            w.add_widget(subwidget2);
        }

        EXPECT_EQ("w1", w.id());
        EXPECT_EQ("expandable", w.widget_type());
        EXPECT_EQ(1u, w.attribute_values().size());
        EXPECT_EQ("foo", w.attribute_values()["title"].get_string());

        auto const widgets = w.widgets();
        EXPECT_EQ(2u, widgets.size());
        auto it = widgets.begin();
        EXPECT_EQ("w2", it->id());
        EXPECT_EQ("src", it->attribute_mappings()["source"]);
        ++it;
        EXPECT_EQ("w3", it->id());
        EXPECT_EQ("baz", it->attribute_values()["source"].get_string());
    }
}

TEST(PreviewWidget, expandable_widget_exceptions)
{
    // cannot have duplicate widget ids
    {
        PreviewWidget w("w1", "expandable");
        PreviewWidget subwidget1("w2", "image");
        PreviewWidget subwidget2("w2", "text");

        w.add_widget(subwidget1);
        EXPECT_THROW(w.add_widget(subwidget2), unity::LogicException);
    }
    // cannot have same id for expandable and sub-widget
    {
        PreviewWidget w("w1", "expandable");
        PreviewWidget subwidget1("w1", "image");

        EXPECT_THROW(w.add_widget(subwidget1), unity::LogicException);
    }
    // cannot add expandable into expandable
    {
        PreviewWidget w("w1", "expandable");
        PreviewWidget subwidget1("w2", "expandable");

        EXPECT_THROW(w.add_widget(subwidget1), unity::LogicException);
    }
}

TEST(PreviewWidget, to_json)
{
    {
        PreviewWidget w("i1", "image");
        w.add_attribute_value("foo", Variant(10));
        w.add_attribute_mapping("boo", "bar");

        internal::JsonCppNode node(w.data());
        EXPECT_EQ("i1", node.get_node("id")->as_string());
        EXPECT_EQ("image", node.get_node("type")->as_string());
        EXPECT_EQ(10, node.get_node("foo")->as_int());
        EXPECT_EQ("bar", node.get_node("components")->get_node("boo")->as_string());
    }
    // json with expandable widget
    {
        PreviewWidget w("w1", "expandable");
        w.add_attribute_value("title", Variant("foo"));

        PreviewWidget subwidget1("w2", "image");
        subwidget1.add_attribute_value("source", Variant("bar"));
        subwidget1.add_attribute_mapping("foo", "booze");

        PreviewWidget subwidget2("w3", "image");
        subwidget2.add_attribute_value("source", Variant("baz"));

        w.add_widget(subwidget1);
        w.add_widget(subwidget2);

        internal::JsonCppNode node(w.data());
        EXPECT_EQ("w1", node.get_node("id")->as_string());
        EXPECT_EQ("foo", node.get_node("title")->as_string());
        EXPECT_EQ("expandable", node.get_node("type")->as_string());
        EXPECT_EQ("w2", node.get_node("widgets")->get_node(0)->get_node("id")->as_string());
        EXPECT_EQ("booze", node.get_node("widgets")->get_node(0)->get_node("components")->get_node("foo")->as_string());
        EXPECT_EQ("w3", node.get_node("widgets")->get_node(1)->get_node("id")->as_string());
    }
}

TEST(PreviewWidget, from_json)
{
    {
        PreviewWidget w(R"({"id": "i1", "type": "header", "title": "foo", "components": {"rating": "boo"}})"); // from json
        EXPECT_EQ("i1", w.id());
        EXPECT_EQ("header", w.widget_type());
        EXPECT_EQ("foo", w.attribute_values()["title"].get_string());
        EXPECT_EQ("boo", w.attribute_mappings()["rating"]);
    }
    // json with expandable widget
    {
        PreviewWidget w(R"({"id": "i1", "type": "expandable", "title": "foo", "components": {"rating": "boo"}, "widgets": [{"id": "w2", "type": "text", "title": "bar"}]})");
        EXPECT_EQ("i1", w.id());
        EXPECT_EQ("expandable", w.widget_type());
        EXPECT_EQ("foo", w.attribute_values()["title"].get_string());
        EXPECT_EQ("boo", w.attribute_mappings()["rating"]);

        auto const widgets = w.widgets();
        EXPECT_EQ(1u, widgets.size());
        auto it = widgets.begin();
        EXPECT_EQ("w2", it->id());
        EXPECT_EQ("text", it->widget_type());
        EXPECT_EQ("bar", it->attribute_values()["title"].get_string());
    }
}

TEST(PreviewWidget, exceptions)
{
    {
        try
        {
            PreviewWidget w("", "image");
            FAIL();
        }
        catch (unity::InvalidArgumentException const&) {}
    }
    {
        try
        {
            PreviewWidget w("id", "");
            FAIL();
        }
        catch (unity::InvalidArgumentException const&) {}
    }
    {
        PreviewWidget w("a", "image");
        EXPECT_THROW(w.add_attribute_value("id", Variant("x")), unity::InvalidArgumentException);
        EXPECT_THROW(w.add_attribute_value("type", Variant("x")), unity::InvalidArgumentException);
        EXPECT_THROW(w.add_attribute_mapping("id", "x"), unity::InvalidArgumentException);
        EXPECT_THROW(w.add_attribute_mapping("type", "x"), unity::InvalidArgumentException);
    }
}

TEST(PreviewWidget, serialize)
{
    {
        PreviewWidget w("i1", "image");
        w.add_attribute_value("foo", Variant(10));
        w.add_attribute_mapping("boo", "bar");

        auto var = w.serialize();
        EXPECT_EQ("i1", var["id"].get_string());
        EXPECT_EQ("image", var["type"].get_string());
        EXPECT_EQ(10, var["attributes"].get_dict()["foo"].get_int());
        EXPECT_EQ("bar", var["components"].get_dict()["boo"].get_string());
    }
    // expandable widget
    {
        PreviewWidget w("w1", "expandable");
        w.add_attribute_value("title", Variant("foo"));

        PreviewWidget subwidget1("w2", "image");
        subwidget1.add_attribute_value("source", Variant("bar"));
        subwidget1.add_attribute_mapping("source", "src");

        PreviewWidget subwidget2("w3", "image");
        subwidget2.add_attribute_value("source", Variant("baz"));

        w.add_widget(subwidget1);
        w.add_widget(subwidget2);

        auto var = w.serialize();
        {
            EXPECT_EQ("w1", var["id"].get_string());
            EXPECT_EQ("expandable", var["type"].get_string());
            EXPECT_EQ("w2", var["widgets"].get_array()[0].get_dict()["id"].get_string());
            EXPECT_EQ("w3", var["widgets"].get_array()[1].get_dict()["id"].get_string());
        }
        // deserialize it back
        {
            PreviewWidget deserialized = internal::PreviewWidgetImpl::create(var);
            EXPECT_EQ("w1", deserialized.id());
            EXPECT_EQ("expandable", deserialized.widget_type());
            EXPECT_EQ("foo", w.attribute_values()["title"].get_string());

            auto const widgets = deserialized.widgets();
            EXPECT_EQ(2u, widgets.size());
            auto it = widgets.begin();
            EXPECT_EQ("w2", it->id());
            EXPECT_EQ("src", it->attribute_mappings()["source"]);
            ++it;
            EXPECT_EQ("w3", it->id());
            EXPECT_EQ("baz", it->attribute_values()["source"].get_string());
        }
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
        EXPECT_EQ(1u, w.attribute_values().size());
        EXPECT_EQ("bar", w.attribute_values()["foo"].get_string());
        EXPECT_EQ(1u, w.attribute_mappings().size());
        EXPECT_EQ("cee", w.attribute_mappings()["bee"]);
    }
}

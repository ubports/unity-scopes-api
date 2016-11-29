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

#include <unity/scopes/qt/QPreviewWidget.h>
#include <unity/scopes/qt/internal/QPreviewWidgetImpl.h>

using namespace unity::scopes::qt;

TEST(QPreviewWidget, bindings)
{
    {
        QPreviewWidget qpw("id", "video");

        EXPECT_EQ("id", qpw.id());
        EXPECT_EQ("video", qpw.widget_type());
    }

    {
        QPreviewWidget qpw(R"({"id": "img", "type": "image", "components": { "source": "screenshot-url" } })");

        EXPECT_EQ("img", qpw.id());
        EXPECT_EQ("image", qpw.widget_type());

        auto am = qpw.attribute_mappings();
        ASSERT_EQ(1, am.size());
        EXPECT_EQ("screenshot-url", am.value("source"));

        auto av = qpw.attribute_values();
        ASSERT_EQ(0, av.size());

        auto widgets = qpw.widgets();
        ASSERT_EQ(0, widgets.size());

        QPreviewWidget qpw2(qpw);

        EXPECT_EQ("img", qpw2.id());
        EXPECT_EQ("image", qpw2.widget_type());

        QPreviewWidget qpw3("xxx", "audio");
        qpw3 = qpw2;
        EXPECT_EQ("img", qpw3.id());
        EXPECT_EQ("image", qpw3.widget_type());

        qpw3 = qpw3;
        EXPECT_EQ("img", qpw3.id());
        EXPECT_EQ("image", qpw3.widget_type());

        QPreviewWidget parent_widget("parent", "expandable");
        parent_widget.add_widget(qpw);
        widgets = parent_widget.widgets();
        ASSERT_EQ(1, widgets.size());
        EXPECT_EQ("img", widgets.at(0).id());

        qpw.add_attribute_value("key", "value");
        av = qpw.attribute_values();
        ASSERT_EQ(1, av.size());
        EXPECT_EQ("value", av.value("key"));

        qpw.add_attribute_mapping("attr", "field");
        am = qpw.attribute_mappings();
        ASSERT_EQ(2, am.size());
        EXPECT_EQ("field", am.value("attr"));
    }

    {
        QString data = R"({"components":{"source":"screenshot-url"},"id":"img","key":"value","type":"image"})";
        QPreviewWidget qpw(data);
        EXPECT_EQ(data + "\n", qpw.data());  // Underlying scopes API appends a newline

        auto vm = qpw.serialize();
        ASSERT_EQ(4, vm.size());
        EXPECT_EQ("img", vm.value("id"));
        EXPECT_EQ("image", vm.value("type"));
        auto attrs = vm.value("attributes").toMap();
        ASSERT_EQ(1, attrs.size());
        EXPECT_EQ("value", attrs.value("key").toString());
        auto components = vm.value("components").toMap();
        ASSERT_EQ(1, components.size());
        EXPECT_EQ("screenshot-url", components.value("source").toString());
    }

    {
        internal::QPreviewWidgetImpl i("id", "audio");
        EXPECT_EQ("id", i.id());
        EXPECT_EQ("audio", i.widget_type());

        internal::QPreviewWidgetImpl i2(i);
        EXPECT_EQ("id", i2.id());
        EXPECT_EQ("audio", i2.widget_type());

        internal::QPreviewWidgetImpl i3("xxx", "video");
        i = i3;
        EXPECT_EQ("xxx", i.id());
        EXPECT_EQ("video", i.widget_type());

        i = i;
        EXPECT_EQ("xxx", i.id());
        EXPECT_EQ("video", i.widget_type());
    }
}

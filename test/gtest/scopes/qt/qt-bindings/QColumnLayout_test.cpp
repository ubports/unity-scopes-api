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

#include <gtest/gtest.h>

#include <unity/scopes/qt/QColumnLayout.h>
#include <unity/scopes/qt/QUtils.h>
#include <unity/scopes/qt/internal/QColumnLayoutImpl.h>

#include <unity/scopes/ColumnLayout.h>

#include <iostream>

using namespace unity::scopes::qt;

TEST(QColumnLayout, bindings)
{
    internal::QColumnLayoutImpl *impl = new internal::QColumnLayoutImpl(2);
    QColumnLayout layout = internal::QColumnLayoutImpl::create(impl);

    // get the internal api layout
    unity::scopes::ColumnLayout * api_layout = impl->get_api();

    // start checking that the internal class has the right attributes
    EXPECT_EQ(api_layout->number_of_columns(), 2);
    EXPECT_EQ(api_layout->number_of_columns(), layout.number_of_columns());

    QVector<QString> qt_widget_ids {"widget1", "widget2"};
    QVector<QString> qt_widget_ids2 {"widget3", "widget4"};
    layout.add_column(qt_widget_ids);
    layout.add_column(qt_widget_ids2);
    QVector<QString> retrieved_widget_ids = layout.column(0);

    std::vector<std::string> api_widgets = api_layout->column(0);
    EXPECT_EQ(api_widgets.size(), retrieved_widget_ids.size());
    EXPECT_TRUE(api_widgets.size() != 0);
    for(uint i = 0; i < api_widgets.size(); ++i)
    {
        EXPECT_EQ(api_widgets[i], retrieved_widget_ids[i].toStdString());
    }

    retrieved_widget_ids = layout.column(1);
    api_widgets = api_layout->column(1);
    EXPECT_EQ(api_widgets.size(), retrieved_widget_ids.size());
    EXPECT_TRUE(api_widgets.size() != 0);
    for(uint i = 0; i < api_widgets.size(); ++i)
    {
        EXPECT_EQ(api_widgets[i], retrieved_widget_ids[i].toStdString());
    }

    EXPECT_EQ(variantmap_to_qvariantmap(api_layout->serialize()), layout.serialize());

    QColumnLayout layout2(2);
    layout2.add_column(qt_widget_ids);
    layout2.add_column(qt_widget_ids2);
    EXPECT_EQ(variantmap_to_qvariantmap(api_layout->serialize()), layout2.serialize());
}

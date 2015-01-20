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

#include <unity/scopes/qt/QPreviewWidget.h>

#include <unity/scopes/qt/internal/QPreviewWidgetImpl.h>

using namespace unity::scopes::qt;

/// @cond
QPreviewWidget::QPreviewWidget(QString const& id, QString const &widget_type)
    : p(new internal::QPreviewWidgetImpl(id.toUtf8().data(), widget_type.toUtf8().data()))
{
}

QPreviewWidget::QPreviewWidget(QString const& definition)
    : p(new internal::QPreviewWidgetImpl(definition.toUtf8().data()))
{
}

QPreviewWidget::QPreviewWidget(QPreviewWidget const& other)
    : p(new internal::QPreviewWidgetImpl(*other.p->api_widget_))
{
}

QPreviewWidget::QPreviewWidget(QPreviewWidget&&) = default;

QPreviewWidget::~QPreviewWidget() = default;

QPreviewWidget& QPreviewWidget::operator=(QPreviewWidget const& other)
{
    if(&other != this)
    {
        p.reset(new internal::QPreviewWidgetImpl(*other.p->api_widget_));
    }
    return *this;
}

QPreviewWidget& QPreviewWidget::operator=(QPreviewWidget&&) = default;

void QPreviewWidget::add_attribute_value(QString const& key, QVariant const& value)
{
    p->add_attribute_value(key, value);
}

void QPreviewWidget::add_attribute_mapping(QString const& key, QString const& field_name)
{
    p->add_attribute_mapping(key, field_name);
}

void QPreviewWidget::add_widget(QPreviewWidget const& widget)
{
    p->add_widget(widget);
}

QString QPreviewWidget::id() const
{
    return p->id();
}

QString QPreviewWidget::widget_type() const
{
    return p->widget_type();
}

QMap<QString, QString> QPreviewWidget::attribute_mappings() const
{
    return p->attribute_mappings();
}

QVariantMap QPreviewWidget::attribute_values() const
{
    return p->attribute_values();
}

QPreviewWidgetList QPreviewWidget::widgets() const
{
    return p->widgets();
}

QString QPreviewWidget::data() const
{
    return p->data();
}

QVariantMap QPreviewWidget::serialize() const
{
    return p->serialize();
}

QPreviewWidget::QPreviewWidget(internal::QPreviewWidgetImpl *widget)
    : p(new internal::QPreviewWidgetImpl(*widget))
{
}
/// @endcond

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

#include <unity/scopes/qt/internal/QPreviewWidgetImpl.h>

#include <unity/scopes/qt/QUtils.h>
#include <unity/scopes/qt/QPreviewWidget.h>

#include <unity/scopes/PreviewWidget.h>

using namespace unity::scopes;
using namespace unity::scopes::qt;
using namespace unity::scopes::qt::internal;

QPreviewWidgetImpl::QPreviewWidgetImpl(QString const& id, QString const& widget_type)
    : api_widget_(new PreviewWidget(id.toUtf8().data(), widget_type.toUtf8().data()))
{
}

QPreviewWidgetImpl::QPreviewWidgetImpl(QString const& definition)
    : api_widget_(new PreviewWidget(definition.toUtf8().data()))
{
}

QPreviewWidgetImpl::QPreviewWidgetImpl(QPreviewWidgetImpl const& other)
    : api_widget_(new PreviewWidget(*other.api_widget_))
{
}

QPreviewWidgetImpl::QPreviewWidgetImpl(QPreviewWidgetImpl&&) = default;

QPreviewWidgetImpl::~QPreviewWidgetImpl() = default;

QPreviewWidgetImpl& QPreviewWidgetImpl::operator=(QPreviewWidgetImpl const& other)
{
    if (&other != this)
    {
        api_widget_.reset(new PreviewWidget(*other.api_widget_));
    }
    return *this;
}

QPreviewWidgetImpl& QPreviewWidgetImpl::operator=(QPreviewWidgetImpl&& other) = default;

void QPreviewWidgetImpl::add_attribute_value(QString const& key, QVariant const& value)
{
    api_widget_->add_attribute_value(key.toUtf8().data(), qVariantToScopeVariant(value));
}

void QPreviewWidgetImpl::add_attribute_mapping(QString const& key, QString const& field_name)
{
    api_widget_->add_attribute_mapping(key.toUtf8().data(), field_name.toUtf8().data());
}

void QPreviewWidgetImpl::add_widget(QPreviewWidget const& widget)
{
    api_widget_->add_widget(*widget.p->api_widget_);
}

QString QPreviewWidgetImpl::id() const
{
    return QString::fromUtf8(api_widget_->id().c_str());
}

QString QPreviewWidgetImpl::widget_type() const
{
    return QString::fromUtf8(api_widget_->widget_type().c_str());
}

QMap<QString, QString> QPreviewWidgetImpl::attribute_mappings() const
{
    QMap<QString, QString> ret_map;
    for (auto item : api_widget_->attribute_mappings())
    {
        ret_map[QString::fromUtf8(item.first.c_str())] = QString::fromUtf8(item.second.c_str());
    }
    return ret_map;
}

QVariantMap QPreviewWidgetImpl::attribute_values() const
{
    return scopeVariantMapToQVariantMap(api_widget_->attribute_values());
}

QPreviewWidgetList QPreviewWidgetImpl::widgets() const
{
    QPreviewWidgetList ret_list;
    for (auto item : api_widget_->widgets())
    {
        ret_list.push_back(QPreviewWidget(new QPreviewWidgetImpl(item)));
    }
    return ret_list;
}

QString QPreviewWidgetImpl::data() const
{
    return QString::fromUtf8(api_widget_->data().c_str());
}

QVariantMap QPreviewWidgetImpl::serialize() const
{
    return scopeVariantMapToQVariantMap(api_widget_->serialize());
}

QPreviewWidgetImpl::QPreviewWidgetImpl(PreviewWidget const& api_widget)
    : api_widget_(new PreviewWidget(api_widget))
{
}

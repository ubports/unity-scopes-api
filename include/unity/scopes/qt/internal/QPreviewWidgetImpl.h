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

#pragma once

#include <unity/util/NonCopyable.h>
#include <unity/util/DefinesPtrs.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <QtCore/QVariant>
#pragma GCC diagnostic pop

namespace unity
{

namespace scopes
{

class PreviewWidget;

namespace qt
{

class QPreviewWidget;

typedef QList<QPreviewWidget> QPreviewWidgetList;

namespace internal
{

class QPreviewReplyImpl;

class QPreviewWidgetImpl
{
public:
    UNITY_DEFINES_PTRS(QPreviewWidgetImpl);

    QPreviewWidgetImpl(QString const& id, QString const& widget_type);
    QPreviewWidgetImpl(QString const& definition);
    virtual ~QPreviewWidgetImpl();

    QPreviewWidgetImpl(QPreviewWidgetImpl const& other);
    QPreviewWidgetImpl(QPreviewWidgetImpl&& other);

    QPreviewWidgetImpl& operator=(QPreviewWidgetImpl const& other);
    QPreviewWidgetImpl& operator=(QPreviewWidgetImpl&& other);

    void add_attribute_value(QString const& key, QVariant const& value);
    void add_attribute_mapping(QString const& key, QString const& field_name);
    void add_widget(QPreviewWidget const& widget);
    QString id() const;
    QString widget_type() const;
    QMap<QString, QString> attribute_mappings() const;
    QVariantMap attribute_values() const;
    QPreviewWidgetList widgets() const;
    QString data() const;

    QVariantMap serialize() const;

private:
    std::unique_ptr<PreviewWidget> api_widget_;
    QPreviewWidgetImpl(PreviewWidget const& api_widget);

    friend class unity::scopes::qt::QPreviewWidget;
    friend class internal::QPreviewReplyImpl;
};

}  // namespace internal

}  // namespace qt

}  // namespace scopes

}  // namespace unity

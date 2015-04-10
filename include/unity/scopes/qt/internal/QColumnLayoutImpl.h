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

#include <unity/scopes/qt/QColumnLayout.h>

#include <unity/util/DefinesPtrs.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <QtCore/QVariant>
#pragma GCC diagnostic pop
#include <QtCore/QVector>

namespace unity
{

namespace scopes
{

class ColumnLayout;

namespace qt
{

namespace internal
{
class QPreviewReplyImpl;

class QColumnLayoutImpl
{
public:
    UNITY_DEFINES_PTRS(QColumnLayoutImpl);

    explicit QColumnLayoutImpl(int num_of_columns);
    ~QColumnLayoutImpl();

    QColumnLayoutImpl(QColumnLayoutImpl const& other);
    QColumnLayoutImpl(QColumnLayoutImpl&&);
    QColumnLayoutImpl& operator=(QColumnLayoutImpl const& other);
    QColumnLayoutImpl& operator=(QColumnLayoutImpl&&);

    void add_column(QVector<QString> widget_ids);
    int size() const noexcept;
    int number_of_columns() const noexcept;
    QVector<QString> column(int index) const;

    QVariantMap serialize() const;

    // added for testing purposes
    static unity::scopes::qt::QColumnLayout create(QColumnLayoutImpl* internal);
    // added for testing purposes
    unity::scopes::ColumnLayout* get_api();

private:
    QColumnLayoutImpl(ColumnLayout* api_layout);
    std::unique_ptr<ColumnLayout> api_layout_;

    friend class unity::scopes::qt::QColumnLayout;
    friend class internal::QPreviewReplyImpl;
};

}  // namespace internal

}  // namespace qt

}  // namespace scopes

}  // namespace unity

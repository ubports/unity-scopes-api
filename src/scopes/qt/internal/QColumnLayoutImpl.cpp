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

#include <unity/scopes/qt/internal/QColumnLayoutImpl.h>

#include <unity/scopes/qt/QUtils.h>

#include <unity/scopes/ColumnLayout.h>

#include <QtCore/QVectorIterator>

#include <vector>

using namespace unity::scopes;
using namespace unity::scopes::qt::internal;

QColumnLayoutImpl::QColumnLayoutImpl(int num_of_columns)
    : api_layout_(new ColumnLayout(num_of_columns))
{
}

QColumnLayoutImpl::QColumnLayoutImpl(QColumnLayoutImpl const& other)
    : api_layout_(new ColumnLayout(*other.api_layout_))
{
}

QColumnLayoutImpl::QColumnLayoutImpl(QColumnLayoutImpl&&) = default;

QColumnLayoutImpl& QColumnLayoutImpl::operator=(QColumnLayoutImpl const& other)
{
    if (&other != this)
    {
        api_layout_.reset(new ColumnLayout(*other.api_layout_));
    }
    return *this;
}

QColumnLayoutImpl& QColumnLayoutImpl::operator=(QColumnLayoutImpl&&) = default;

QColumnLayoutImpl::~QColumnLayoutImpl() = default;

void QColumnLayoutImpl::add_column(QVector<QString> widget_ids)
{
    std::vector<std::string> api_list;
    QVectorIterator<QString> it(widget_ids);
    while (it.hasNext())
    {
        api_list.push_back(it.next().toUtf8().data());
    }
    api_layout_->add_column(api_list);
}

int QColumnLayoutImpl::size() const noexcept
{
    return api_layout_->size();
}

int QColumnLayoutImpl::number_of_columns() const noexcept
{
    return api_layout_->number_of_columns();
}

QVector<QString> QColumnLayoutImpl::column(int index) const
{
    QVector<QString> ret_vector;
    for (auto item : api_layout_->column(index))
    {
        ret_vector.push_back(QString::fromUtf8(item.c_str()));
    }

    return ret_vector;
}

QVariantMap QColumnLayoutImpl::serialize() const
{
    return scopeVariantMapToQVariantMap(api_layout_->serialize());
}

QColumnLayoutImpl::QColumnLayoutImpl(ColumnLayout* api_layout)
    : api_layout_(new ColumnLayout(*api_layout))
{
}

// added for testing purposes
unity::scopes::qt::QColumnLayout QColumnLayoutImpl::create(QColumnLayoutImpl *internal)
{
    return QColumnLayout(internal);
}
// added for testing purposes
unity::scopes::ColumnLayout * QColumnLayoutImpl::get_api()
{
    return api_layout_.get();
}

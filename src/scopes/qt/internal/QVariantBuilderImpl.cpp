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

#include <unity/scopes/qt/internal/QVariantBuilderImpl.h>

#include <unity/scopes/qt/QUtils.h>

#include <unity/scopes/VariantBuilder.h>

using namespace unity::scopes;
using namespace unity::scopes::qt;
using namespace unity::scopes::qt::internal;

QVariantBuilderImpl::QVariantBuilderImpl()
    : api_builder_(new VariantBuilder())
{
}

QVariantBuilderImpl::QVariantBuilderImpl(QVariantBuilderImpl const& other)
    : api_builder_(new VariantBuilder(*(other.api_builder_)))
{
}

void QVariantBuilderImpl::add_tuple(std::initializer_list<QPair<QString, QVariant>> const& tuple)
{
    std::vector<std::pair<std::string, Variant>> aux_vector;
    for (auto item : tuple)
    {
        // convert every item to std...
        aux_vector.push_back(std::make_pair(item.first.toUtf8().data(), qVariantToScopeVariant(item.second)));
    }
    api_builder_->add_tuple(aux_vector);
}

void QVariantBuilderImpl::add_tuple(QVector<QPair<QString, QVariant>> const& tuple)
{
    std::vector<std::pair<std::string, Variant>> aux_vector;
    for (auto item : tuple)
    {
        // convert every item to std...
        aux_vector.push_back(std::make_pair(item.first.toUtf8().data(), qVariantToScopeVariant(item.second)));
    }
    api_builder_->add_tuple(aux_vector);
}

QVariant QVariantBuilderImpl::end()
{
    return scopeVariantToQVariant(api_builder_->end());
}

QVariantBuilder QVariantBuilderImpl::create(QVariantBuilderImpl* internal)
{
    return QVariantBuilder(internal);
}

unity::scopes::VariantBuilder* QVariantBuilderImpl::get_api()
{
    return api_builder_.get();
}

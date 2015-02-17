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

#include <unity/scopes/qt/internal/QCategoryImpl.h>
#include <unity/scopes/qt/QCategory.h>
#include <unity/scopes/qt/QUtils.h>

using namespace unity::scopes;
using namespace unity::scopes::qt;
using namespace unity::scopes::qt::internal;

QCategoryImpl::QCategoryImpl(unity::scopes::Category::SCPtr category)
    : api_category_(category)
{
}

QCategoryImpl::~QCategoryImpl()
{
}

QString QCategoryImpl::id() const
{
    return QString::fromStdString(api_category_->id());
}

QString QCategoryImpl::title() const
{
    return QString::fromStdString(api_category_->title());
}

QString QCategoryImpl::icon() const
{
    return QString::fromStdString(api_category_->icon());
}

CannedQuery::SCPtr QCategoryImpl::query() const
{
    return api_category_->query();
}

CategoryRenderer const& QCategoryImpl::renderer_template() const
{
    return api_category_->renderer_template();
}

QVariantMap QCategoryImpl::serialize() const
{
    QVariantMap ret_map;
    for (auto items : api_category_->serialize())
    {
        ret_map[QString::fromStdString(items.first)] = variant_to_qvariant(items.second);
    }
    return ret_map;
}

std::shared_ptr<QCategory> QCategoryImpl::create(unity::scopes::Category::SCPtr category)
{
    std::shared_ptr<QCategory> ret(new QCategory(new QCategoryImpl(category)));
    return ret;
}

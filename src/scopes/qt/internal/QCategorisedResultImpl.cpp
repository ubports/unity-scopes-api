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

#include <unity/scopes/qt/internal/QCategorisedResultImpl.h>
#include <unity/scopes/qt/internal/QCategoryImpl.h>

#include <unity/scopes/qt/QCategory.h>

#include <unity/scopes/CategorisedResult.h>

#include <cassert>

using namespace std;
using namespace unity::scopes;
using namespace unity::scopes::qt;
using namespace unity::scopes::qt::internal;

QCategorisedResultImpl::QCategorisedResultImpl(QCategory::SCPtr category)
    : QResultImpl(new CategorisedResult(category->p->api_category_))
{
}

QCategorisedResultImpl::QCategorisedResultImpl(QCategorisedResultImpl const& other)
    : QResultImpl(new CategorisedResult(*other.api_object()))
{
}

QCategorisedResultImpl& QCategorisedResultImpl::operator=(QCategorisedResultImpl const& other)
{
    api_result_.reset(new CategorisedResult(*other.api_object()));
    return *this;
}

QCategorisedResultImpl::QCategorisedResultImpl(QCategorisedResultImpl&&) = default;

void QCategorisedResultImpl::set_category(QCategory::SCPtr category)
{
    api_object()->set_category(category->p->api_category_);
}

QCategory::SCPtr QCategorisedResultImpl::category() const
{
    std::shared_ptr<QCategory> ret_category(new QCategory(api_object()->category()));
    return ret_category;
}

CategorisedResult* QCategorisedResultImpl::api_object() const
{
    sync_values();
    CategorisedResult* categorisedResult = dynamic_cast<CategorisedResult*>(api_result_.get());
    assert(categorisedResult);
    return categorisedResult;
}

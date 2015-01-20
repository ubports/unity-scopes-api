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

#include <unity/scopes/qt/internal/QSearchReplyImpl.h>
#include <unity/scopes/qt/internal/QCategorisedResultImpl.h>
#include <unity/scopes/qt/internal/QDepartmentImpl.h>

#include <unity/scopes/qt/QCategorisedResult.h>
#include <unity/scopes/qt/QDepartment.h>

#include <unity/scopes/CategorisedResult.h>
#include <unity/scopes/Category.h>

using namespace unity::scopes;
using namespace unity::scopes::qt;
using namespace unity::scopes::qt::internal;

QSearchReplyImpl::QSearchReplyImpl(unity::scopes::SearchReplyProxy& reply)
    : api_reply_(reply)
{
}

QSearchReplyImpl::~QSearchReplyImpl()
{
}

void QSearchReplyImpl::register_departments(QDepartment::SCPtr const& parent)
{
    api_reply_->register_departments(parent->p->api_department_);
}

QCategory::SCPtr QSearchReplyImpl::register_category(QString const& id,
                                                     QString const& title,
                                                     QString const& icon,
                                                     unity::scopes::CategoryRenderer const& renderer_template)
{
    Category::SCPtr category = api_reply_->register_category(
        id.toUtf8().data(), title.toUtf8().data(), icon.toUtf8().data(), renderer_template);
    std::shared_ptr<QCategory> ret_category(new QCategory(category));
    return ret_category;
}

bool QSearchReplyImpl::push(QCategorisedResult const& result)
{
    return api_reply_->push(*result.fwd()->api_object());
}

void QSearchReplyImpl::error(std::exception_ptr ex)
{
    api_reply_->error(ex);
}

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

#include <unity/scopes/qt/QSearchReply.h>
#include <unity/scopes/qt/internal/QSearchReplyImpl.h>

using namespace unity::scopes::qt;
namespace sc = unity::scopes;

QSearchReply::QSearchReply(unity::scopes::SearchReplyProxy& reply, QObject *parent)
    : QObject(parent), p(new internal::QSearchReplyImpl(reply))
{
}

QSearchReply::~QSearchReply()
{
}

void QSearchReply::register_departments(QDepartment::SCPtr const& parent)
{
    p->register_departments(parent);
}

QCategory::SCPtr QSearchReply::register_category(QString const& id,
                                                 QString const& title,
                                                 QString const& icon,
                                                 sc::CategoryRenderer const& renderer_template)
{
    // convert QStrings to std::string
    return p->register_category(id.toUtf8().data(), title.toUtf8().data(), icon.toUtf8().data(), renderer_template);
}

bool QSearchReply::push(QCategorisedResult const& result)
{
    return p->push(result);
}

void QSearchReply::error(std::exception_ptr ex)
{
    p->error(ex);
}

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

#include <unity/util/DefinesPtrs.h>
#include <unity/util/NonCopyable.h>

#include <unity/scopes/SearchReply.h>
#include <unity/scopes/SearchReplyProxyFwd.h>

#include <unity/scopes/qt/QCategory.h>
#include <unity/scopes/qt/QDepartment.h>

#include <QtCore/QString>

namespace unity
{

namespace scopes
{

class SearchMetadata;
class CannedQuery;

namespace qt
{

class QSearchReply;
class QCategorisedResult;

namespace internal
{

class QSearchReplyImpl
{
    friend class unity::scopes::qt::QSearchReply;

public:
    NONCOPYABLE(QSearchReplyImpl);
    UNITY_DEFINES_PTRS(QSearchReplyImpl);

    virtual ~QSearchReplyImpl();
    virtual void register_departments(QDepartment::SCPtr const& parent);
    QCategory::SCPtr register_category(
        QString const& id,
        QString const& title,
        QString const& icon,
        unity::scopes::CategoryRenderer const& renderer_template = unity::scopes::CategoryRenderer());
    bool push(QCategorisedResult const& result);
    void error(std::exception_ptr ex);

protected:
    unity::scopes::SearchReplyProxy api_reply_;

private:
    QSearchReplyImpl(unity::scopes::SearchReplyProxy& reply);
};

}  // namespace internal

}  // namespace qt

}  // namespace scopes

}  // namespace unity

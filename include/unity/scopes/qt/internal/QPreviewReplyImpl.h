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

#include <unity/scopes/PreviewWidget.h>
#include <unity/scopes/PreviewReplyProxyFwd.h>

#include <unity/scopes/qt/QColumnLayout.h>
#include <unity/scopes/qt/QPreviewWidget.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <QtCore/QVariant>
#pragma GCC diagnostic pop

namespace unity
{

namespace scopes
{

class PreviewReply;

namespace qt
{

class QPreviewReply;

namespace internal
{

class QPreviewReplyImpl
{
public:
    NONCOPYABLE(QPreviewReplyImpl);
    UNITY_DEFINES_PTRS(QPreviewReplyImpl);

    virtual ~QPreviewReplyImpl();

    bool register_layout(QColumnLayoutList const& layouts);
    bool push(QPreviewWidgetList const& widget_list);
    bool push(QString const& key, QVariant const& value);

protected:
    QPreviewReplyImpl(unity::scopes::PreviewReplyProxy& reply);

private:
    PreviewReplyProxy api_reply_;

    friend class unity::scopes::qt::QPreviewReply;
};

}  // namespace internal

}  // namespace qt

}  // namespace scopes

}  // namespace unity

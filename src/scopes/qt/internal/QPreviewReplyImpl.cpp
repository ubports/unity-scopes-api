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

#include <unity/scopes/qt/internal/QPreviewReplyImpl.h>
#include <unity/scopes/qt/internal/QColumnLayoutImpl.h>
#include <unity/scopes/qt/internal/QPreviewWidgetImpl.h>

#include <unity/scopes/qt/QUtils.h>

#include <unity/scopes/ColumnLayout.h>
#include <unity/scopes/PreviewReply.h>

#include <QtCore/QListIterator>

using namespace unity::scopes;
using namespace unity::scopes::qt::internal;

bool QPreviewReplyImpl::register_layout(QColumnLayoutList const& layouts)
{
    ColumnLayoutList api_list;
    QListIterator<QColumnLayout> it(layouts);

    while (it.hasNext())
    {
        api_list.push_back(ColumnLayout(*it.next().p->api_layout_));
    }
    return api_reply_->register_layout(api_list);
}

bool QPreviewReplyImpl::push(QPreviewWidgetList const& widget_list)
{
    PreviewWidgetList api_list;
    QListIterator<QPreviewWidget> it(widget_list);
    while (it.hasNext())
    {
        api_list.push_back(PreviewWidget(*it.next().p->api_widget_));
    }
    return api_reply_->push(api_list);
}

bool QPreviewReplyImpl::push(QString const& key, QVariant const& value)
{
    return api_reply_->push(key.toUtf8().data(), qVariantToScopeVariant(value));
}

QPreviewReplyImpl::~QPreviewReplyImpl() = default;

QPreviewReplyImpl::QPreviewReplyImpl(unity::scopes::PreviewReplyProxy& reply)
    : api_reply_(reply)
{
}

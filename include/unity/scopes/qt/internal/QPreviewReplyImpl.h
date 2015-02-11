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

/**
\brief Allows the results of a preview to be sent to the preview requester.
*/

class QPreviewReplyImpl
{
public:
    /// @cond
    NONCOPYABLE(QPreviewReplyImpl);
    UNITY_DEFINES_PTRS(QPreviewReplyImpl);
    /// @endcond
    /**
    \brief Registers a list of column layouts for the current preview.

    Layouts must be registered before pushing a unity::scopes::PreviewWidgetList, and must be
    registered only once.
    \return True if the query is still alive, false if the query failed or was cancelled.
    \throws unity::LogicException register_layout() is called more than once.
    */
    bool register_layout(QColumnLayoutList const& layouts);

    /**
    \brief Sends widget definitions to the sender of the preview query.

    This method can be called mutiple times to send widgets in stages.
    \return True if the query is still alive, false if the query failed or was cancelled.
    */
    bool push(QPreviewWidgetList const& widget_list);

    /**
    \brief Sends data for a preview widget attribute.
    \return True if the query is still alive, false if the query failed or was cancelled.
    */
    bool push(QString const& key, QVariant const& value);

    /// @cond
    virtual ~QPreviewReplyImpl();
    /// @endcond

protected:
    /// @cond
    QPreviewReplyImpl(unity::scopes::PreviewReplyProxy& reply);
    /// @endcond

private:
    PreviewReplyProxy api_reply_;

    friend class unity::scopes::qt::QPreviewReply;
};

}  // namespace internal

}  // namespace qt

}  // namespace scopes

}  // namespace unity

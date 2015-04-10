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

#ifndef _ENABLE_QT_EXPERIMENTAL_
#error You should define _ENABLE_QT_EXPERIMENTAL_ in order to use this experimental header file.
#endif

#include <unity/util/DefinesPtrs.h>
#include <unity/util/NonCopyable.h>

#include <unity/scopes/PreviewQueryBase.h>
#include <unity/scopes/PreviewReplyProxyFwd.h>
#include <unity/scopes/qt/QPreviewQueryBase.h>

#include <QtCore/QObject>

class QCoreApplication;

namespace unity
{

namespace scopes
{

class Result;
class ActionMetadata;

namespace qt
{

namespace tests
{
class QPreviewQueryBaseAPIMock;
}

namespace internal
{
class QScopeBaseAPIImpl;
}

class QScopeBase;
class QScopeBaseAPI;

/**
\brief Abstract base class to represent a particular preview.

A scope must return an instance of this class from its implementation of ScopeBase::preview().

This is the class that links scope API calls with the main QThread.
The instance of this class is moved to the main QThread and pushes events to the Qt event loop.

\note The constructor of the instance must complete in a timely manner. Do not perform anything in the
constructor that might block.
*/
class QPreviewQueryBaseAPI : public QObject, public unity::scopes::PreviewQueryBase
{
    Q_OBJECT
public:
    /// @cond
    NONCOPYABLE(QPreviewQueryBaseAPI);
    UNITY_DEFINES_PTRS(QPreviewQueryBaseAPI);

    virtual ~QPreviewQueryBaseAPI();

protected:
    QPreviewQueryBaseAPI(std::shared_ptr<QCoreApplication> qtapp,
                         QScopeBase& qtscope,
                         unity::scopes::Result const& result,
                         unity::scopes::ActionMetadata const& metadata,
                         QObject* parent = 0);

    bool event(QEvent* e) override;
    /// @endcond

    /**
    \brief Called by scopes run time to start the query.

    Your implementation of run() can use the reply proxy to push results
    for the query. You can push results from within run(), in which case
    the query implicitly completes when run() returns. Alternatively,
    run() can store the reply proxy and return immediately. In this
    case, you can use the stored proxy to push results from another
    thread. It is safe to call `push()` from multiple threads without
    synchronization.

    The query completes either when run() returns, or when the
    last stored reply proxy goes out of scope (whichever happens
    last).

    \param reply The proxy on which to push results for the query.
    */
    virtual void run(unity::scopes::PreviewReplyProxy const& reply) final;

    /**
    \brief Called by the scopes run time when the query originator
    cancels a query.

    Your implementation of this method should ensure that the scope stops
    processing the current query as soon as possible. Any calls to a `push()` method
    once a query is cancelled are ignored, so continuing to push after cancellation
    only wastes CPU cycles. (`push()` returns `false` once a query is cancelled or
    exceeds its cardinality limit.)
    */
    virtual void cancelled() final;  // Originator cancelled the query

    /// @cond
    std::shared_ptr<QCoreApplication> qtapp_;
    std::unique_ptr<QPreviewQueryBase> qtquery_;
    QScopeBase& qtscope_;

private:
    void init();

    friend internal::QScopeBaseAPIImpl;
    friend  unity::scopes::qt::tests::QPreviewQueryBaseAPIMock;
    /// @endcond
};

}  // namespace qt

}  // namespace scopes

}  // namespace unity

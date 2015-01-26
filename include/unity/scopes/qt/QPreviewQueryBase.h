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

#include <unity/scopes/PreviewReplyProxyFwd.h>

#include <unity/scopes/qt/QActionMetadata.h>
#include <unity/scopes/qt/QCannedQuery.h>
#include <unity/scopes/qt/QPreviewReplyProxy.h>
#include <unity/scopes/qt/QResult.h>

#include <QtCore/QObject>

namespace unity
{

namespace scopes
{

class SearchMetadata;
class CannedQuery;
class ActionMetadata;

namespace qt
{

class QResult;

namespace internal
{
class QPreviewQueryBaseImpl;
}

class QPreviewQueryBaseAPI;

/**
\brief Abstract base class to represent a particular preview.

A scope must return an instance of this class from its implementation of ScopeBase::preview().

\note The constructor of the instance must complete in a timely manner. Do not perform anything in the
constructor that might block.
*/
class QPreviewQueryBase : public QObject
{
    Q_OBJECT
public:
    /// @cond
    NONCOPYABLE(QPreviewQueryBase);
    UNITY_DEFINES_PTRS(QPreviewQueryBase);
    /// @endcond

    virtual ~QPreviewQueryBase();

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
    virtual void run(unity::scopes::qt::QPreviewReplyProxy const& reply) = 0;

    /**
    \brief Called by the scopes run time when the query originator
    cancels a query.

    Your implementation of this method should ensure that the scope stops
    processing the current query as soon as possible. Any calls to a `push()` method
    once a query is cancelled are ignored, so continuing to push after cancellation
    only wastes CPU cycles. (`push()` returns `false` once a query is cancelled or
    exceeds its cardinality limit.)
    */
    virtual void cancelled() = 0;  // Originator cancelled the query

    /**
     \brief Get result for this preview request.
     \throws unity::LogicException if result was not initialized (the default ctor was used).
     \return result
     */
    QResult result() const;

    /**
     \brief Get metadata for this preview request.
     \return search metadata
     \throws unity::LogicException if preview metadata was not initialized (the default ctor was used).
    */
    QActionMetadata action_metadata() const;

protected:
    /// @cond
    QPreviewQueryBase(QObject* parent = 0);

private:
    void init(QPreviewQueryBaseAPI* query_api);

    std::unique_ptr<internal::QPreviewQueryBaseImpl> p;
    friend class internal::QPreviewQueryBaseImpl;
    friend class QPreviewQueryBaseAPI;
    /// @endcond
};

}  // namespace qt

}  // namespace scopes

}  // namespace unity

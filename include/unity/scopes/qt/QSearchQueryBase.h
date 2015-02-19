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
#include <unity/scopes/SearchListenerBase.h>
#include <unity/scopes/QueryCtrlProxyFwd.h>

#include <unity/scopes/qt/QSearchReplyProxy.h>
#include <unity/scopes/qt/QCannedQuery.h>

#include <QtCore/QObject>

namespace unity
{

namespace scopes
{

class SearchMetadata;
class CannedQuery;

namespace qt
{

namespace internal
{
class QSearchQueryBaseImpl;
}

class QSearchQueryBaseAPI;

/**
\brief Abstract base class to represent a particular query.

A scope must return an instance of this class from its implementation of ScopeBase::create_query().

\note The constructor of the instance must complete in a timely manner. Do not perform anything in the
constructor that might block.
*/
class QSearchQueryBase : public QObject
{
    Q_OBJECT
public:
    /// @cond
    NONCOPYABLE(QSearchQueryBase);
    UNITY_DEFINES_PTRS(QSearchQueryBase);
    /// @endcond

    virtual ~QSearchQueryBase();

    /**
     \brief Get a canned query for this search request.

     \return The canned query.
     \throws unity::LogicException if the canned query was not initialized (was default-constructed).
     */
    QCannedQuery query() const;

    /**
     \brief Get metadata for this search request.
     \return The search metadata.
     \throws unity::LogicException if search metadata was not initialized (was default-constructed).
    */
    unity::scopes::SearchMetadata search_metadata() const;

    /** @name Subquery methods
    The subsearch() methods are for use by aggregating scopes.
    When an aggregator passes a query to its child scopes, it should
    use subsearch() instead of the normal Scope::search()
    that would be called by a client. subsearch() takes care
    of automatically forwarding query cancellation to child scopes.
    This means that there is no need for an aggregating scope to
    explicitly forward cancellation to child scopes
    when its QueryBase::cancelled() method is called by the scopes
    run time.
    */
    //{@
    QueryCtrlProxy subsearch(ScopeProxy const& scope,
                             std::string const& query_string,
                             SearchListenerBase::SPtr const& reply);
    QueryCtrlProxy subsearch(ScopeProxy const& scope,
                             std::string const& query_string,
                             FilterState const& filter_state,
                             SearchListenerBase::SPtr const& reply);
    QueryCtrlProxy subsearch(ScopeProxy const& scope,
                             std::string const& query_string,
                             std::string const& department_id,
                             FilterState const& filter_state,
                             SearchListenerBase::SPtr const& reply);
    QueryCtrlProxy subsearch(ScopeProxy const& scope,
                             std::string const& query_string,
                             std::string const& department_id,
                             FilterState const& filter_state,
                             SearchMetadata const& hints,
                             SearchListenerBase::SPtr const& reply);

public Q_SLOTS:
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
    virtual void run(QSearchReplyProxy const& reply) = 0;

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

protected:
    ///@cond
    QSearchQueryBase(QObject* parent = 0);

private:
    void init(QSearchQueryBaseAPI* query_api);

    std::unique_ptr<internal::QSearchQueryBaseImpl> p;
    friend class internal::QSearchQueryBaseImpl;
    friend class QSearchQueryBaseAPI;
    ///@endcond
};

}  // namespace qt

}  // namespace scopes

}  // namespace unity

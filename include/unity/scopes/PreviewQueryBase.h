/*
 * Copyright (C) 2013 Canonical Ltd
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
 * Authored by: Michi Henning <michi.henning@canonical.com>
 */

#pragma once

#include <unity/scopes/PreviewReplyProxyFwd.h>
#include <unity/scopes/QueryCtrlProxyFwd.h>
#include <unity/scopes/ScopeProxyFwd.h>
#include <unity/scopes/Variant.h>
#include <unity/scopes/QueryBase.h>

#include <unity/util/DefinesPtrs.h>
#include <unity/util/NonCopyable.h>

namespace unity
{

namespace scopes
{

class Result;
class ActionMetadata;

namespace internal
{

class PreviewQueryBaseImpl;
class QueryBaseImpl;
class QueryObject;

} // namespace internal

/**
\brief Abstract base class to represent a particular preview.

A scope must return an instance of this class from its implementation of ScopeBase::preview().

\note The constructor of the instance must complete in a timely manner. Do not perform anything in the
constructor that might block.
*/

class PreviewQueryBase : public QueryBase
{
public:
    /// @cond
    NONCOPYABLE(PreviewQueryBase);
    UNITY_DEFINES_PTRS(PreviewQueryBase);
    /// @endcond

    /**
    \brief Called by scopes run time to start the preview.

    Your implementation of run() can use the reply proxy to push results
    for the preview. You can push results from within run(), in which case
    the preview implicitly completes when run() returns. Alternatively,
    run() can store the reply proxy and return immediately. In this
    case, you can use the stored proxy to push results from another
    thread. It is safe to call `push()` from multiple threads without
    synchronization.

    The preview completes either when run() returns, or when the
    last stored reply proxy goes out of scope (whichever happens
    last).

    \param reply The proxy on which to push results for the preview.
    */
    virtual void run(PreviewReplyProxy const& reply) = 0;         // Called by the run time to start this preview

    /**
     \brief Get result for this preview request.
     \throws unity::LogicException if result was not initialized (the default ctor was used).
     \return result
     */
    Result result() const;

    /**
     \brief Get metadata for this preview request.
     \return search metadata
     \throws unity::LogicException if preview metadata was not initialized (the default ctor was used).
    */
    ActionMetadata action_metadata() const;

    // TODO: Add a method for subpreview request?

    /// @cond
    virtual ~PreviewQueryBase();
    /// @endcond

protected:
    /**
    \brief Instantiates a PreviewQueryBase.
    \param result The result to be previewed.
    \param metadata Additional data for the preview.
    */
    PreviewQueryBase(Result const& result, ActionMetadata const& metadata);

private:
    internal::PreviewQueryBaseImpl* fwd() const;
};

} // namespace scopes

} // namespace unity

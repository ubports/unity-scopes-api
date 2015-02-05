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

#include <unity/scopes/ScopeBase.h>
#include <unity/scopes/QueryBase.h>
#include <unity/scopes/ReplyProxyFwd.h>
#include <unity/scopes/QueryBase.h>
#include <unity/scopes/PreviewQueryBase.h>

#include <unity/scopes/qt/QScopeBase.h>

#include <memory>
#include <thread>

namespace unity
{

namespace scopes
{

namespace qt
{

class QScopeBase;

namespace internal
{
class QScopeBaseAPIImpl;
}

/**
 * Defines the lifecycle of scope plugin, and acts as a factory
 * for Query and Preview objects.
 *
 * Note that the #preview and #search methods are each called on
 * different threads, so some form of interlocking is required
 * if shared data structures are used.
 */
class QScopeBaseAPI : public unity::scopes::ScopeBase
{
public:
    /// @cond
    using FactoryFunc = std::function<QScopeBase*()>;

    NONCOPYABLE(QScopeBaseAPI);
    UNITY_DEFINES_PTRS(QScopeBaseAPI);

    QScopeBaseAPI(FactoryFunc const& creator);
    virtual ~QScopeBaseAPI() = default;
    /// @endcond

    /**
    \brief Called by the scopes run time after the create function completes.

    If start() throws an exception, stop() will _not_ be called.

    The call to start() is made by the same thread that calls the create function.

    \param scope_id The name of the scope as defined by the scope's configuration file.
    */
    virtual void start(std::string const& scope_id);

    /**
    \brief Called by the scopes run time when the scope should shut down.

    A scope should deallocate as many resources as possible when stop() is called, for example,
    deallocate any caches and close network connections. In addition, if the scope implements run()
    and did not return from run(), it must return from run() in response to the call to stop().

    Exceptions from stop() are ignored.

    The call to stop() is made by the same thread that calls the create function and start().
    */
    virtual void stop();

    /**
     * Called each time a new preview is requested
     */
    virtual unity::scopes::PreviewQueryBase::UPtr preview(const unity::scopes::Result&,
                                                          const unity::scopes::ActionMetadata&) override;

    /**
     * Called each time a new query is requested
     */
    virtual unity::scopes::SearchQueryBase::UPtr search(unity::scopes::CannedQuery const& q,
                                                        unity::scopes::SearchMetadata const&) override;

private:
    /// @cond
    std::unique_ptr<internal::QScopeBaseAPIImpl> p;
    friend class internal::QScopeBaseAPIImpl;
    /// @endcond
};

}  // namespace qt

}  // namespace scopes

}  // namespace unity

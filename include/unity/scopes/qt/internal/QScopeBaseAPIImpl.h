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

#include <unity/scopes/QueryBase.h>
#include <unity/scopes/ReplyProxyFwd.h>
#include <unity/scopes/SearchQueryBase.h>
#include <unity/scopes/PreviewQueryBase.h>

#include <QtCore/QObject>

#include <thread>
#include <memory>
#include <atomic>

class QCoreApplication;

namespace unity
{

namespace scopes
{

namespace qt
{

class QScopeBase;

namespace internal
{

class QScopeBaseAPIImpl : public QObject
{
    Q_OBJECT
public:
    /// @cond
    using FactoryFunc = std::function<QScopeBase*()>;

    NONCOPYABLE(QScopeBaseAPIImpl);
    UNITY_DEFINES_PTRS(QScopeBaseAPIImpl);

    QScopeBaseAPIImpl(QScopeBase& qtscope, QObject* parent = 0);
    QScopeBaseAPIImpl(FactoryFunc const& creator, QObject* parent = 0);
    virtual ~QScopeBaseAPIImpl();

    bool event(QEvent* e) override;
    /// @endcond

    /**
    \brief Called by the scopes run time after the create function completes.

    If start() throws an exception, stop() will _not_ be called.

    The call to start() is made by the same thread that calls the create function.

    \param scope_id The name of the scope as defined by the scope's configuration file.
    */
    void start(std::string const& scope_id);

    /**
    \brief Called by the scopes run time when the scope should shut down.

    A scope should deallocate as many resources as possible when stop() is called, for example,
    deallocate any caches and close network connections. In addition, if the scope implements run()
    and did not return from run(), it must return from run() in response to the call to stop().

    Exceptions from stop() are ignored.

    The call to stop() is made by the same thread that calls the create function and start().
    */
    void stop();

    /**
     * Called each time a new preview is requested
     */
    unity::scopes::PreviewQueryBase::UPtr preview(const unity::scopes::Result&, const unity::scopes::ActionMetadata&);

    /**
     * Called each time a new query is requested
     */
    unity::scopes::SearchQueryBase::UPtr search(unity::scopes::CannedQuery const& q,
                                                unity::scopes::SearchMetadata const&);

protected:
    void startQtThread();

    std::shared_ptr<QCoreApplication> qtapp_;
    std::unique_ptr<std::thread> qtthread_;
    std::atomic<bool> qtapp_ready_;

    QScopeBase* qtscope_impl_;

    FactoryFunc qtscope_creator_;
};

}  // namespace internal

}  // namespace qt

}  // namespace scopes

}  // namespace unity

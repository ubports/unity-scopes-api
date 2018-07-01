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

#include <unity/scopes/QueryBase.h>
#include <unity/scopes/ReplyProxyFwd.h>
#include <unity/scopes/SearchQueryBase.h>
#include <unity/scopes/PreviewQueryBase.h>

#include <QtCore/QObject>

#include <functional>
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
    using FactoryFunc = std::function<QScopeBase*()>;

    NONCOPYABLE(QScopeBaseAPIImpl);
    UNITY_DEFINES_PTRS(QScopeBaseAPIImpl);

    QScopeBaseAPIImpl(FactoryFunc const& creator);
    virtual ~QScopeBaseAPIImpl();

    bool event(QEvent* e) override;
    void start(std::string const& scope_id);
    void stop();
    unity::scopes::PreviewQueryBase::UPtr preview(const unity::scopes::Result&, const unity::scopes::ActionMetadata&);
    unity::scopes::SearchQueryBase::UPtr search(unity::scopes::CannedQuery const& q,
                                                unity::scopes::SearchMetadata const&);

protected:
    void start_qt_thread();

    std::shared_ptr<QCoreApplication> qtapp_;
    std::unique_ptr<std::thread> qtthread_;
    std::atomic<bool> qtapp_ready_;
    std::atomic<bool> qtapp_stopped_;
    std::unique_ptr<QScopeBase> qtscope_impl_;

    FactoryFunc qtscope_creator_;
};

}  // namespace internal

}  // namespace qt

}  // namespace scopes

}  // namespace unity

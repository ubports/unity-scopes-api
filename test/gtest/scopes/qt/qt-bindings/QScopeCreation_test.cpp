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

#include "FakeScope.h"
#include <gtest/gtest.h>

#include <unity/scopes/qt/QScopeBaseAPI.h>
#include <unity/scopes/qt/QUtils.h>

#include <unity/scopes/qt/internal/QScopeBaseAPIImpl.h>

#include <QtCore/QThread>
#include <QtCore/QCoreApplication>

#include <atomic>
#include <iostream>

using namespace unity::scopes::qt::internal;
using namespace unity::scopes::qt;
using namespace unity::scopes;

QThread* qt_thread = nullptr;

// variable to control when the user's scope
// has been created
std::atomic<bool> scope_created(false);

class QScopeBaseAPIImplTest : public QScopeBaseAPIImpl
{
public:
    QScopeBaseAPIImplTest(FactoryFunc const& creator, QObject* parent = 0)
        : QScopeBaseAPIImpl(creator, parent)
    {
    }

    ~QScopeBaseAPIImplTest()
    {
        delete qtscope_impl_;
    }

    QThread* getQtAppThread()
    {
        return qtapp_->thread();
    }

    bool qtapp_stopped()
    {
        return qtapp_stopped_;
    }
};

QScopeBase* create_my_scope()
{
    // get the current thread for further use
    qt_thread = QThread::currentThread();
    scope_created = true;
    return new QScope();
}

TEST(CreationTest, bindings)
{
    QScopeBaseAPIImplTest impl(create_my_scope);

    // start the Qt application
    // this creates the qt instance
    impl.start("test_scope");

    // wait for the user's scope to be created
    // As the scope is created in the Qt event loop
    // we have to wait until the event has been processed
    std::chrono::milliseconds dura(10);
    while (!scope_created)
    {
        std::this_thread::sleep_for(dura);
    }

    // check that the thread of the qt app is the
    // same than the one filled in the creation function
    EXPECT_EQ(qt_thread, impl.getQtAppThread());

    // stop the Qt application
    impl.stop();

    while (!impl.qtapp_stopped())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

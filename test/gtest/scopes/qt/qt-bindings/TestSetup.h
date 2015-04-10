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

#include <gtest/gtest.h>

#include <QtCore/QCoreApplication>
#include <QtCore/QEvent>
#include <QtCore/QThread>

#include <memory>
#include <thread>
#include <atomic>

class TestSetup : public testing::Test
{
public:
    TestSetup()
        : qtapp_(nullptr)
        , thread_id_(nullptr)
    {
    }

    virtual ~TestSetup()
    {
        std::cout << "~TestSetup()" << std::endl;
    }

    virtual void SetUp()
    {
        std::cout << "SetUp()" << std::endl;
        qtthread_ = std::unique_ptr<std::thread>(new std::thread(&TestSetup::startQtThread, this));
        while (!thread_id_)
        {
            std::chrono::milliseconds dura(10);
            std::this_thread::sleep_for(dura);
        }
    }

    virtual void TearDown()
    {
        std::cout << "TearDown()" << std::endl;
        if (qtapp_)
        {
            std::cout << "Stopping qt application" << std::endl;
            qtapp_->quit();
        }
        qtthread_->join();
    }

    void startQtThread()
    {
        std::cout << "Starting qt application" << std::endl;
        int argc = 0;
        char* argv = NULL;
        qtapp_ = std::make_shared<QCoreApplication>(argc, &argv);
        std::cout << "Thread id is: " << QThread::currentThreadId() << std::endl;
        thread_id_ = QThread::currentThreadId();
        qtapp_->exec();
        std::cout << "Application finished" << std::endl;
        // Destroy the QCoreApplication from the same thread
        qtapp_.reset();
    }

    std::shared_ptr<QCoreApplication> qtapp_;
    std::unique_ptr<std::thread> qtthread_;
    std::atomic<void*> thread_id_;
};

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

#include "FakeScope.h"
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <QtCore/QThread>

#include <mutex>

class QScopeMock : public QScope
{
public:
    QScopeMock()
        : QScope()
        , qt_thread_(nullptr){};

    void set_qt_thread(QThread* thread)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        qt_thread_ = thread;
    }

    MOCK_METHOD1(start, void(QString const&));
    MOCK_METHOD0(stop, void());
    virtual unity::scopes::qt::QPreviewQueryBase::UPtr preview(const unity::scopes::qt::QResult&,
                                                               const unity::scopes::qt::QActionMetadata&) override
    {
        std::lock_guard<std::mutex> lock(mutex_);
        EXPECT_EQ(QThread::currentThread(), qt_thread_);
        return unity::scopes::qt::QPreviewQueryBase::UPtr(new QPreview());
    }

    virtual unity::scopes::qt::QSearchQueryBase::UPtr search(unity::scopes::CannedQuery const&,
                                                             unity::scopes::SearchMetadata const&) override
    {
        std::lock_guard<std::mutex> lock(mutex_);
        EXPECT_EQ(QThread::currentThread(), qt_thread_);
        return unity::scopes::qt::QSearchQueryBase::UPtr(new QQuery());
    }

protected:
    QThread* qt_thread_;
    std::mutex mutex_;
};

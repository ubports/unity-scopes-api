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
#include <gmock/gmock.h>

#include <unity/scopes/qt/internal/QScopeBaseAPIImpl.h>

#include <QtCore/QCoreApplication>
#include <QtCore/QThread>

class QScopeBaseAPIMock : public unity::scopes::qt::internal::QScopeBaseAPIImpl
{
public:
    QScopeBaseAPIMock(unity::scopes::qt::QScopeBase& qtscope, QObject *parent=0)
                            : unity::scopes::qt::internal::QScopeBaseAPIImpl(qtscope, parent)
    {
    }

    virtual ~QScopeBaseAPIMock() = default;

    QThread *getQtAppThread()
    {
        return qtapp_->thread();
    }
};

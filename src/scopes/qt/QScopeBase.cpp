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

#include <unity/scopes/qt/QScopeBase.h>
#include <unity/scopes/qt/internal/QScopeBaseImpl.h>

using namespace unity::scopes::qt;

///@cond
QScopeBase::QScopeBase(QObject* parent)
    : QObject(parent)
    , p(new internal::QScopeBaseImpl())
{
}

QScopeBase::~QScopeBase()
{
}
///@endcond

void QScopeBase::start(QString const& start_string)
{
    p->start(start_string.toUtf8().data());
}

void QScopeBase::stop()
{
    p->stop();
}

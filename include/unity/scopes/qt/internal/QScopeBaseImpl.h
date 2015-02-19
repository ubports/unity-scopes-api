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

#include <unity/scopes/qt/QSearchQueryBase.h>

#include <unity/util/DefinesPtrs.h>
#include <unity/util/NonCopyable.h>

namespace unity
{

namespace scopes
{

namespace qt
{

namespace internal
{

class QScopeBaseImpl
{
public:
    /// @cond
    NONCOPYABLE(QScopeBaseImpl);
    UNITY_DEFINES_PTRS(QScopeBaseImpl);

    QScopeBaseImpl() = default;
    virtual ~QScopeBaseImpl() = default;
    /// @endcond
    /**
     * Called once at startup
     */
    virtual void start(std::string const&);

    /**
     * Called at shutdown
     */
    virtual void stop();
};

}  // namespace internal

}  // namespace qt

}  // namespace scopes

}  // namespace unity

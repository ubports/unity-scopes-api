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

#include <unity/scopes/qt/QActionMetadata.h>
#include <unity/scopes/qt/QCannedQuery.h>
#include <unity/scopes/qt/QResult.h>

namespace unity
{

namespace scopes
{

class SearchMetadata;
class CannedQuery;

namespace qt
{

class QPreviewQueryBase;
class QPreviewQueryBaseAPI;

namespace internal
{

class QPreviewQueryBaseImpl
{
public:
    /// @cond
    NONCOPYABLE(QPreviewQueryBaseImpl);
    UNITY_DEFINES_PTRS(QPreviewQueryBaseImpl);
    /// @endcond

    QPreviewQueryBaseImpl();
    ~QPreviewQueryBaseImpl() = default;

    /**
     \brief Get result for this preview request.
     \throws unity::LogicException if result was not initialized (the default ctor was used).
     \return result
     */
    QResult result() const;

    /**
     \brief Get metadata for this preview request.
     \return search metadata
     \throws unity::LogicException if preview metadata was not initialized (the default ctor was used).
    */
    QActionMetadata action_metadata() const;

protected:
    QPreviewQueryBaseAPI* preview_api_;

private:
    void init(QPreviewQueryBaseAPI* preview_api);

    friend class unity::scopes::qt::QPreviewQueryBase;
};

}  // namespace internal

}  // namespace qt

}  // namespace scopes

}  // namespace unity

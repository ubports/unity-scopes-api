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

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <QtCore/QVariantMap>
#pragma GCC diagnostic pop
#include <QtCore/QString>

namespace unity
{

namespace scopes
{

class Variant;

namespace qt
{

namespace internal
{
class QResultImpl;
class QSearchMetadataImpl;
class QActionMetadataImpl;

/// @cond
class QScopeVariant : public QVariant
{
public:
    //    explicit QScopeVariant() noexcept;
    virtual ~QScopeVariant();

    QScopeVariant(QScopeVariant const& other);
    QScopeVariant(QScopeVariant&&);

    QScopeVariant& operator=(QScopeVariant const& other);
    QScopeVariant& operator=(QScopeVariant&&);

    QScopeVariant& operator=(int val) noexcept;
    QScopeVariant& operator=(double val) noexcept;
    QScopeVariant& operator=(bool val) noexcept;
    QScopeVariant& operator=(QString const& val);
    QScopeVariant& operator=(char const* val);  // Required to prevent v = "Hello" from storing a bool
    QScopeVariant& operator=(QVariantMap const& val);
    virtual QScopeVariant& operator=(QVariant const& val);

    std::string get_string() const;

private:
    /**
    \brief Creates a Variant instance that stores the supplied integer.
    */
    explicit QScopeVariant(Variant* val) noexcept;

    void setInternalVariant(Variant* val);

    void sync();
    friend class internal::QResultImpl;
    friend class internal::QSearchMetadataImpl;
    friend class internal::QActionMetadataImpl;

    Variant* internal_variant_;
    QVariant ret_variant;
};
/// @endcond

}  // namespace internal

}  // namespace qt

}  // namespace scopes

}  // namespace unity

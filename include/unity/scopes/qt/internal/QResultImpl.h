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

#include <unity/scopes/ScopeProxyFwd.h>

#include <QtCore/QString>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <QtCore/QVariantMap>
#pragma GCC diagnostic pop

class QVariant;

namespace unity
{

namespace scopes
{

class Result;
class Variant;

namespace qt
{

class QResult;
class QCategorisedResult;

namespace internal
{

class QScopeVariant;
class QCategorisedResultImpl;

class QResultImpl
{
public:
    UNITY_DEFINES_PTRS(QResultImpl);

    QResultImpl(unity::scopes::Result const& other);
    QResultImpl(QResultImpl const& other);

    QResultImpl& operator=(QResultImpl const& other);
    QResultImpl& operator=(QResultImpl&&);

    virtual ~QResultImpl();

    void store(QResult const& other, bool intercept_activation = false);
    bool has_stored_result() const;
    QResult retrieve() const;
    void set_uri(QString const& uri);
    void set_title(QString const& title);
    void set_art(QString const& image);
    void set_dnd_uri(QString const& dnd_uri);
    void set_intercept_activation();
    bool direct_activation() const;
    ScopeProxy target_scope_proxy() const;
    QVariant& operator[](QString const& key);
    QVariant const& operator[](QString const& key) const;
    QString uri() const noexcept;
    QString title() const noexcept;
    QString art() const noexcept;
    QString dnd_uri() const noexcept;
    bool contains(QString const& key) const;
    QVariant const& value(QString const& key) const;
    QVariantMap serialize() const;

protected:
    explicit QResultImpl(const QVariantMap& variant_map);
    explicit QResultImpl(internal::QResultImpl* impl);
    explicit QResultImpl(unity::scopes::Result* api_result);

    std::unique_ptr<unity::scopes::Result> api_result_;

private:
    void sync_values() const;

    mutable QMap<QString, std::shared_ptr<QScopeVariant>> ret_variants_;
    mutable QList<std::shared_ptr<QScopeVariant>> unsync_variants_;

    friend class unity::scopes::qt::QResult;
    friend class QCategorisedResultImpl;
    friend class unity::scopes::qt::QCategorisedResult;
};

}  // namespace internal

}  // namespace qt

}  // namespace scopes

}  // namespace unity

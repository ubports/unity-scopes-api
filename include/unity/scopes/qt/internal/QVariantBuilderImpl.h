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

#include <QtCore/QPair>
#include <QtCore/QString>
#include <QtCore/QVariant>
#include <QtCore/QVector>

#include <memory>

namespace unity
{

namespace scopes
{

class VariantBuilder;

namespace qt
{

namespace internal
{

class QVariantBuilderImpl
{
public:
    QVariantBuilderImpl();
    QVariantBuilderImpl(QVariantBuilderImpl const& other);
    QVariantBuilderImpl(QVariantBuilderImpl&&) = default;
    ~QVariantBuilderImpl() = default;
    QVariantBuilderImpl& operator=(QVariantBuilderImpl const& other) = delete;
    QVariantBuilderImpl& operator=(QVariantBuilderImpl&&) = default;
    void add_tuple(std::initializer_list<QPair<QString, QVariant>> const& tuple);
    void add_tuple(QVector<QPair<QString, QVariant>> const& tuple);
    QVariant end();

private:
    std::unique_ptr<unity::scopes::VariantBuilder> api_builder_;
};

}  // namespace internal

}  // namespace qt

}  // namespace scopes

}  // namespace unity

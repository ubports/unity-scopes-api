/*
 * Copyright (C) 2014 Canonical, Ltd.
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of version 3 of the GNU Lesser General Public License as published
 * by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Xavi Garcia <xavi.garcia.mena@canonical.com>
 */
#pragma once

#include <QtCore/QJsonDocument>
#include <QtCore/QUrl>

#include <memory>
#include <vector>

namespace unity
{

namespace scopes
{

namespace utils
{

namespace qt
{

class JsonAsyncReader;

class JsonReader final
{
public:
    JsonReader();
    ~JsonReader() = default;

    typedef std::shared_ptr<QJsonDocument> QJsonDocumentSptr;

    typedef QVector<QPair<QString, QString>> QJsonParameters;

    // Method provided for general use.
    // The user should parse the Json root.
    QJsonDocumentSptr read(QUrl const& uri) const;

    QJsonDocumentSptr read(QString const& host, QJsonParameters const& parameters) const;

private:
    JsonReader(JsonReader const&) = delete; /** Deleted */
    JsonReader& operator=(JsonReader const&) = delete;

    std::shared_ptr<JsonAsyncReader> p_;
};

}  // namespace qt

}  // namespace utils

}  // namespace scopes

}  // namespace unity

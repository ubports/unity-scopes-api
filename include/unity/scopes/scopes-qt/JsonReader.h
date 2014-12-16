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

#ifndef _ENABLE_QT_EXPERIMENTAL_
    #error You should define _ENABLE_QT_EXPERIMENTAL_ in order to use this experimental header file.
#endif

#include <QtCore/QJsonDocument>
#include <QtCore/QUrl>

#include <memory>
#include <vector>

namespace unity
{

namespace scopes
{

namespace qt
{

class JsonAsyncReader;

/**
\brief Class that downloads http JSON files synchronously.

Executes a remote HTTP query synchronously to return a JSON parser filled with the downloaded data.
*/
class JsonReader final
{
public:
    /// @cond
    JsonReader();
    ~JsonReader() = default;

    typedef std::shared_ptr<QJsonDocument> QJsonDocumentSptr;

    typedef QVector<QPair<QString, QString>> QJsonParameters;
    /// @endcond

    /**
     * \brief Downloads a remote JSON document and returns a parser containing the data.
     * \param uri URI to download
     *
     * \return shared pointer of a QJsonDocument containing the downloaded data.
     */
    QJsonDocumentSptr read(QUrl const& uri) const;

    /**
     * \brief Downloads a remote JSON document and returns a parser containing the data.
     * \param host the remote host name
     * \param parameters The parameters that will build the final query, defined by a list of pairs of key and value
     *
     * \return shared pointer of a QJsonDocument containing the downloaded data.
     */
    QJsonDocumentSptr read(QString const& host, QJsonParameters const& parameters) const;

private:
    JsonReader(JsonReader const&) = delete; /** Deleted */
    JsonReader& operator=(JsonReader const&) = delete;

    std::shared_ptr<JsonAsyncReader> p_;
};

}  // namespace qt

}  // namespace scopes

}  // namespace unity

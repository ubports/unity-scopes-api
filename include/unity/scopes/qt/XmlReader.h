/*
 * Copyright (C) 2014 Canonical Ltd
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
 * Author: Xavi Garcia <xavi.garcia.mena@canonical.com>
 */

#pragma once

#ifndef _ENABLE_QT_EXPERIMENTAL_
#error You should define _ENABLE_QT_EXPERIMENTAL_ in order to use this experimental header file.
#endif

#include <QtCore/QUrl>
#include <QtCore/QXmlStreamReader>

#include <memory>
#include <vector>

namespace unity
{

namespace scopes
{

namespace qt
{

class XmlAsyncReader;

/**
\brief Class that downloads http XML files synchronously.

Executes a remote HTTP query synchronously to return a XML parser filled with the downloaded data.
*/
class XmlReader final
{
public:
    /// @cond
    XmlReader();
    ~XmlReader() = default;

    typedef std::shared_ptr<QXmlStreamReader> QXmlStreamReaderSptr;

    typedef QVector<QPair<QString, QString>> QXmlStreamReaderParameters;
    /// @endcond

    /**
     * \brief Downloads a remote XML document and returns a parser containing the data.
     * \param uri URI to download
     *
     * \return shared pointer of a QXmlStreamReader containing the downloaded data.
     */
    QXmlStreamReaderSptr read(QUrl const& uri) const;

    /**
     * \brief Downloads a remote XML document and returns a parser containing the data.
     * \param host the remote host name
     * \param parameters The parameters that will build the final query, defined by a list of pairs of key and value
     *
     * \return shared pointer of a QXmlStreamReader containing the downloaded data.
     */
    QXmlStreamReaderSptr read(QString const& host, QXmlStreamReaderParameters const& parameters) const;

private:
    XmlReader(XmlReader const&) = delete; /** Deleted */
    XmlReader& operator=(XmlReader const&) = delete;

    std::shared_ptr<XmlAsyncReader> p_;
};

}  // namespace qt

}  // namespace scopes

}  // namespace unity

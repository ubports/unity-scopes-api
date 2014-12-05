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

#include <QtCore/QUrl>
#include <QtCore/QXmlStreamReader>

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

class XmlAsyncReader;
class XmlReader final
{
public:
    XmlReader();
    ~XmlReader() = default;

    typedef std::shared_ptr<QXmlStreamReader> QXmlStreamReaderSptr;

    typedef QVector<QPair<QString, QString>> QXmlStreamReaderParameters;

    // Method provided for general use.
    // The user should parse the Json root.
    QXmlStreamReaderSptr read(QUrl const& uri) const;

    QXmlStreamReaderSptr read(QString const& host, QXmlStreamReaderParameters const& parameters) const;

private:
    XmlReader(XmlReader const&) = delete; /** Deleted */
    XmlReader& operator=(XmlReader const&) = delete;

    std::shared_ptr<XmlAsyncReader> p_;
};

}  // namespace qt

}  // namespace utils

}  // namespace scopes

}  // namespace unity

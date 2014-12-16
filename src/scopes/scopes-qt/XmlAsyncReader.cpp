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

// allow experimental headers
#define _ENABLE_QT_EXPERIMENTAL_

#include <unity/scopes/scopes-qt/XmlAsyncReader.h>

using namespace std;
using namespace unity::scopes::qt;

XmlAsyncReader::XmlAsyncReader()
    : p_(new HttpAsyncReader)
{
}

XmlAsyncReader::QXmlStreamReaderSptr XmlAsyncReader::create_parser_with_data(std::string const& data,
                                                                             std::string& error)
{
    XmlAsyncReader::QXmlStreamReaderSptr nullRes;
    QString qData(data.c_str());
    XmlAsyncReader::QXmlStreamReaderSptr res(new QXmlStreamReader(std::move(qData)));
    if (res->hasError())
    {
        error = res->errorString().toStdString();
        res = nullRes;
    }
    return res;
}

XmlAsyncReader::QXmlStreamReaderFuture XmlAsyncReader::async_get_parser(std::string const& uri) const
{
    return p_->async_get_parser<QXmlStreamReader>(uri, XmlAsyncReader::create_parser_with_data);
}

XmlAsyncReader::QXmlStreamReaderFuture XmlAsyncReader::async_get_parser(
    std::string const& host, XmlAsyncReader::QXmlStreamReaderParams const& params) const
{
    std::string uri = p_->get_uri(host, params);
    return p_->async_get_parser<QXmlStreamReader>(uri, XmlAsyncReader::create_parser_with_data);
}

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

#include <unity/scopes/qt/XmlAsyncReader.h>
#include <unity/scopes/qt/XmlReader.h>

using namespace unity::scopes::qt;

const static int64_t SECONDS_TIMEOUT = 10;

XmlReader::XmlReader()
    : p_(new XmlAsyncReader)
{
}

// Method provided for general use.
// The user should parse the XML root.
XmlReader::QXmlStreamReaderSptr XmlReader::read(QUrl const& uri) const
{
    XmlReader::QXmlStreamReaderSptr item;
    try
    {
        auto item_future = p_->async_get_parser(uri.toString().toStdString());
        item = HttpAsyncReader::get_or_throw(item_future, SECONDS_TIMEOUT);
    }
    catch (unity::scopes::TimeoutException& e)
    {
        // we catch the exception because we don't want to provide
        // any exception related to futures in this API and also to provide
        // more error information, like the uri.
        throw unity::scopes::TimeoutException("XmlReader::read: HTTP request timeout after " +
                                              std::to_string(SECONDS_TIMEOUT) + " seconds. (uri = " +
                                              uri.toString().toStdString() + ")");
    }

    return item;
}

XmlReader::QXmlStreamReaderSptr XmlReader::read(QString const& host,
                                                XmlReader::QXmlStreamReaderParameters const& parameters) const
{
    // convert parameters to std
    std::vector<std::pair<std::string, std::string>> stdParams;
    for (auto param : parameters)
    {
        std::pair<std::string, std::string> stdParam(param.first.toStdString(), param.second.toStdString());
        stdParams.push_back(stdParam);
    }

    XmlReader::QXmlStreamReaderSptr item;
    try
    {
        auto item_future = p_->async_get_parser(host.toStdString(), stdParams);
        item = HttpAsyncReader::get_or_throw(item_future, SECONDS_TIMEOUT);
    }
    catch (unity::scopes::TimeoutException& e)
    {
        // we catch the exception because we don't want to provide
        // any exception related to futures in this API and also to provide
        // more error information, like the uri.

        // construct the error message containing all the parameters and host
        std::stringstream strStream;
        strStream << "XmlReader::read: HTTP request timeout after " << std::to_string(SECONDS_TIMEOUT)
                  << " seconds. (host = " << host.toStdString() << " parameters = { ";
        for (auto param : stdParams)
        {
            strStream << "[" << param.first << ", " << param.second << "] ";
        }
        strStream << "} )";

        throw unity::scopes::TimeoutException(strStream.str());
    }

    return item;
}

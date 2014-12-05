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

#include <unity/util/DefinesPtrs.h>
#include <unity/util/NonCopyable.h>

#include <QtCore/QXmlStreamReader>

#include <future>
#include <memory>
#include "HttpAsyncReader.h"

namespace unity
{

namespace scopes
{

namespace utils
{

namespace qt
{

template <typename TYPE>
using ResultsList = std::deque<std::shared_ptr<TYPE>>;

template <typename B, typename T>
static bool get_results(QXmlStreamReader& xml,
                        const std::string& object_name,
                        ResultsList<B>& results,
                        std::string& error_string);

class XmlAsyncReader
{
public:
    NONCOPYABLE(XmlAsyncReader);
    UNITY_DEFINES_PTRS(XmlAsyncReader);

    template <typename T>
    using ParserFunc =
        std::function<bool(QXmlStreamReader& root, const std::string&, std::deque<std::shared_ptr<T>>&, std::string&)>;

    template <typename T>
    using ResultsFuture = std::future<std::deque<std::shared_ptr<T>>>;

    template <typename TYPE>
    using ResultsList = std::deque<std::shared_ptr<TYPE>>;

    typedef std::future<std::shared_ptr<QXmlStreamReader>> QXmlStreamReaderFuture;

    typedef std::shared_ptr<QXmlStreamReader> QXmlStreamReaderSptr;

    typedef std::vector<std::pair<std::string, std::string>> QXmlStreamReaderParams;

    XmlAsyncReader();
    virtual ~XmlAsyncReader() = default;

    template <typename BASE, typename TYPE>
    ResultsFuture<BASE> async_get(std::string const& uri,
                                  std::string const& object_name,
                                  ParserFunc<BASE> const& parse = get_results<BASE, TYPE>) const;

    template <typename T>
    ResultsFuture<T> async_get(std::string const& uri,
                               std::string const& object_name,
                               ParserFunc<T> const& parse = get_results<T, T>) const;

    template <typename BASE, typename TYPE>
    ResultsFuture<BASE> async_get(std::string const& host,
                                  QXmlStreamReaderParams const& params,
                                  std::string const& object_name,
                                  ParserFunc<BASE> const& parse = get_results<BASE, TYPE>) const;

    template <typename T>
    ResultsFuture<T> async_get(std::string const& host,
                               std::vector<std::pair<std::string, std::string>> const& params,
                               std::string const& object_name,
                               ParserFunc<T> const& parse = get_results<T, T>) const;

    QXmlStreamReaderFuture async_get_parser(std::string const& uri) const;

    QXmlStreamReaderFuture async_get_parser(std::string const& host, QXmlStreamReaderParams const& params) const;

protected:
    static QXmlStreamReaderSptr create_parser_with_data(std::string const& data, std::string& error);

    std::shared_ptr<HttpAsyncReader> p_;
};

template <typename BASE, typename TYPE>
XmlAsyncReader::ResultsFuture<BASE> XmlAsyncReader::async_get(std::string const& uri,
                                                              std::string const& object_name,
                                                              ParserFunc<BASE> const& parse) const
{
    return p_->async_get<BASE, TYPE, QXmlStreamReader>(
        uri, object_name, XmlAsyncReader::create_parser_with_data, parse);
}

template <typename T>
XmlAsyncReader::ResultsFuture<T> XmlAsyncReader::async_get(std::string const& uri,
                                                           std::string const& object_name,
                                                           ParserFunc<T> const& parse) const
{
    return p_->async_get<T, T, QXmlStreamReader>(uri, object_name, XmlAsyncReader::create_parser_with_data, parse);
}

template <typename BASE, typename TYPE>
XmlAsyncReader::ResultsFuture<BASE> XmlAsyncReader::async_get(std::string const& host,
                                                              QXmlStreamReaderParams const& params,
                                                              std::string const& object_name,
                                                              ParserFunc<BASE> const& parse) const
{
    std::string uri = p_->get_uri(host, params);
    return p_->async_get<BASE, TYPE, QXmlStreamReader>(
        uri, object_name, XmlAsyncReader::create_parser_with_data, parse);
}

template <typename T>
XmlAsyncReader::ResultsFuture<T> XmlAsyncReader::async_get(std::string const& host,
                                                           QXmlStreamReaderParams const& params,
                                                           std::string const& object_name,
                                                           ParserFunc<T> const& parse) const
{
    std::string uri = p_->get_uri(host, params);
    return p_->async_get<T, T, QXmlStreamReader>(uri, object_name, XmlAsyncReader::create_parser_with_data, parse);
}

template <typename B, typename T>
static bool get_results(QXmlStreamReader& xml,
                        std::string const& object_name,
                        ResultsList<B>& results,
                        std::string& error_string)
{
    while (!xml.atEnd() && !xml.hasError())
    {
        /* Read next element.*/
        QXmlStreamReader::TokenType token = xml.readNext();
        /* If token is just StartDocument, we'll go to next.*/
        if (token == QXmlStreamReader::StartDocument)
        {
            continue;
        }
        /* If token is StartElement, we'll see if we can read it.*/
        if (token == QXmlStreamReader::StartElement)
        {
            /* If it's named persons, we'll go to the next.*/
            if (xml.name() == QString(object_name.c_str()))
            {
                results.emplace_back(std::make_shared<T>(xml));
            }
        }
    }
    if (xml.hasError())
    {
        error_string = "get_results: ERROR: " + xml.errorString().toStdString() + " at line: " + std::to_string(10);
    }
    /* Error handling. */
    return !xml.hasError();
}

}  // namespace qt

}  // namespace utils

}  // namespace scopes

}  // namespace unity

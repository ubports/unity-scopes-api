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

/// @cond
template <typename TYPE>
using ResultsList = std::deque<std::shared_ptr<TYPE>>;

template <typename B, typename T>
static bool get_results(QXmlStreamReader& xml,
                        const std::string& object_name,
                        ResultsList<B>& results,
                        std::string& error_string);
/// @endcond

/**
\brief Class that downloads http XML files asynchronously.

Executes a remote HTTP query asynchronously to return different futures about XML results or a XML parser.
The class implements and defines a function that returns a XML parser filled with the data
downloaded.
The class also offers a default function to obtain all objects of a given name in the results.
The user may define his own parsing function to parse data at he desires.
*/
class XmlAsyncReader
{
public:
    /// @cond
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
    /// @endcond

    /**
     * \brief Downloads a HTTP XML remote file asynchronously and returns a future to a list of results
     * This method downloads in a separated thread a http document identified by the given URI.
     * It returns a future of list of results based on a given object name.
     * If, for example, the document contains a list of objects identified by the tag "ITEM" it
     * parses the document and returns a list of those objects.
     *
     * The user may provide his own function to parse the downloaded data.
     *
     * The method has 2 template parameters: the type of results returned and the type of objects when instantiating.
     * The type of instantiation has to be a derived class of the return type. This is offered for convenience
     * to return more generic types and get advantage of polymorphism.
     *
     * The method checks at compile time that the instantiation type is effectively a derived class of the type
     * returned.
     *
     * \param uri URI to download
     * \param object_name name of the kind of object we are looking for in the http document
     * \param parse Function that parses the data downloaded
     *
     * \return Future of list of results
     */
    template <typename BASE, typename TYPE>
    ResultsFuture<BASE> async_get(std::string const& uri,
                                  std::string const& object_name,
                                  ParserFunc<BASE> const& parse = get_results<BASE, TYPE>) const;

    /**
     * \brief Downloads a XML HTTP remote file asynchronously and returns a future to a list of results
     * This method downloads in a separated thread a http document identified by the given URI.
     * It returns a future of list of results based on a given object name.
     * If, for example, the document contains a list of objects identified by the tag "ITEM" it
     * parses the document and returns a list of those objects.
     * The user must provide a function to create the parser for the downloaded data and a parse function
     * that retrieves the data.
     *
     * The method has 1 template parameter: the type of objects returned and the parser type.
     *
     * \param uri URI to download
     * \param object_name name of the kind of object we are looking for in the http document
     * \param parse Function that parses the data downloaded
     *
     * \return Future of list of results
     */
    template <typename T>
    ResultsFuture<T> async_get(std::string const& uri,
                               std::string const& object_name,
                               ParserFunc<T> const& parse = get_results<T, T>) const;

    /**
     * \brief Downloads a HTTP XML remote file asynchronously and returns a future to a list of results
     * This method downloads in a separated thread a http document identified by the given URI.
     * It returns a future of list of results based on a given object name.
     * If, for example, the document contains a list of objects identified by the tag "ITEM" it
     * parses the document and returns a list of those objects.
     *
     * The user may provide his own function to parse the downloaded data.
     *
     * The method has 2 template parameters: the type of results returned and the type of objects when intantiating.
     * The type of instantiation has to be a derived class of the return type. This is offered for convenience
     * to return more generic types and get advantage of polymorphism.
     *
     * The method checks at compile time that the instantiation type is effectively a derived class of the type
     * returned.
     *
     * \param host the remote host name
     * \param params The parameters that will build the final query, defined by a list of pairs of key and value
     * \param object_name name of the kind of object we are looking for in the http document
     * \param parse Function that parses the data downloaded
     *
     * \return Future of list of results
     */
    template <typename BASE, typename TYPE>
    ResultsFuture<BASE> async_get(std::string const& host,
                                  QXmlStreamReaderParams const& params,
                                  std::string const& object_name,
                                  ParserFunc<BASE> const& parse = get_results<BASE, TYPE>) const;

    /**
     * \brief Downloads a HTTP XML remote file asynchronously and returns a future to a list of results
     * This method downloads in a separated thread a http document identified by the given URI.
     * It returns a future of list of results based on a given object name.
     * If, for example, the document contains a list of objects identified by the tag "ITEM" it
     * parses the document and returns a list of those objects.
     *
     * The user may provide his own function to parse the downloaded data.
     *
     * The method has 1 template parameters: the type of results returned.
     *
     * The method checks at compile time that the instantiation type is effectively a derived class of the type
     * returned.
     *
     * \param host the remote host name
     * \param params The parameters that wlll build the final query, defined by a list of pairs of key and value
     * \param object_name name of the kind of object we are looking for in the http document
     * \param parse Function that parses the data downloaded
     *
     * \return Future of list of results
     */
    template <typename T>
    ResultsFuture<T> async_get(std::string const& host,
                               QXmlStreamReaderParams const& params,
                               std::string const& object_name,
                               ParserFunc<T> const& parse = get_results<T, T>) const;

    /**
     * \brief Downloads a HTTP remote file asynchronously and returns a future to a valid XML parser containing the
     *data.
     *
     * This method downloads a remote http document, fills a valid QXmlStreamReader parser with the downloaded data and
     * returns a future to the parser.
     * \param uri URI to download
     *
     * \return Future of valid parser filled with the data downloaded
     */
    QXmlStreamReaderFuture async_get_parser(std::string const& uri) const;

    /**
      * \brief Downloads a HTTP remote file asynchronously and returns a future to a valid XML parser containing the
      *data.
      *
      * This method downloads a remote http document, fills a valid QXmlStreamReader parser with the downloaded data and
      * returns a future to the parser.
      * \param host host name
      * \param params The parameters that will build the final query, defined by a list of pairs of key and value
      *
      * \return Future of valid parser filled with the data downloaded
      */
    QXmlStreamReaderFuture async_get_parser(std::string const& host, QXmlStreamReaderParams const& params) const;

protected:
    /**
     * \brief Creates a QXmlStreamReader filled with the given data.
     * \param data The data that contains the XML document
     * \param error Returned by the method, contains any possible error string obtained during the creation of the
     *parser.
     *
     * \return shared pointer to the QXmlStreamReader created
     */
    static QXmlStreamReaderSptr create_parser_with_data(std::string const& data, std::string& error);

    /// @cond
    std::shared_ptr<HttpAsyncReader> p_;
    /// @endcond
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

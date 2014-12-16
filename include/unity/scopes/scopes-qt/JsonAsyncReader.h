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

#include <unity/scopes/scopes-qt/HttpAsyncReader.h>
#include <unity/util/DefinesPtrs.h>
#include <unity/util/NonCopyable.h>

#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QStringList>
#include <QtCore/QVector>

#include <memory>
#include <future>

namespace unity
{

namespace scopes
{

namespace qt
{

template <typename TYPE>
using ResultsList = std::deque<std::shared_ptr<TYPE>>;

template <typename B, typename T>
static bool get_results_json(QJsonDocument& json,
                             std::string const& object_name,
                             ResultsList<B>& results,
                             std::string&);

/**
\brief Class that downloads http JSON files asynchronously.

Executes a remote HTTP query asynchronously to return different futures about JSON results or a JSON parser.
The class implements and defines a function that returns a JSON parser filled with the data
downloaded.
The class also offers a default function to obtain all objects of a given name in the results.
The user may define his own parsing function to parse data at he desires.
*/
class JsonAsyncReader
{
public:
    /// @cond
    NONCOPYABLE(JsonAsyncReader);
    UNITY_DEFINES_PTRS(JsonAsyncReader);

    template <typename B>
    using ParseFunc =
        std::function<bool(QJsonDocument& root, const std::string&, std::deque<std::shared_ptr<B>>&, std::string&)>;

    template <typename T>
    using ResultsFuture = std::future<std::deque<std::shared_ptr<T>>>;

    template <typename TYPE>
    using ResultsList = std::deque<std::shared_ptr<TYPE>>;

    typedef std::future<std::shared_ptr<QJsonDocument>> JsonDocumentFuture;

    typedef std::shared_ptr<QJsonDocument> QJsonDocumentSptr;

    typedef std::vector<std::pair<std::string, std::string>> JsonParameters;

    JsonAsyncReader();

    virtual ~JsonAsyncReader() = default;
    /// @endcond

    /**
     * \brief Downloads a HTTP JSON remote file asynchronously and returns a future to a list of results
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
                                  ParseFunc<BASE> const& parse = get_results_json<BASE, TYPE>) const;

    /**
     * \brief Downloads a JSON HTTP remote file asynchronously and returns a future to a list of results
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
                               ParseFunc<T> const& parse = get_results_json<T, T>) const;

    /**
     * \brief Downloads a HTTP JSON remote file asynchronously and returns a future to a list of results
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
     * \param params The parameters that wlll build the final query, defined by a list of pairs of key and value
     * \param object_name name of the kind of object we are looking for in the http document
     * \param parse Function that parses the data downloaded
     *
     * \return Future of list of results
     */
    template <typename BASE, typename TYPE>
    ResultsFuture<BASE> async_get(std::string const& host,
                                  JsonParameters const& params,
                                  std::string const& object_name,
                                  ParseFunc<BASE> const& parse = get_results_json<BASE, TYPE>) const;

    /**
     * \brief Downloads a HTTP JSON remote file asynchronously and returns a future to a list of results
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
                               JsonParameters const& params,
                               std::string const& object_name,
                               ParseFunc<T> const& parse = get_results_json<T, T>) const;

    /**
     * \brief Downloads a HTTP remote file asynchronously and returns a future to a valid JSON parser containing the
     *data.
     *
     * This method downloads a remote http document, fills a valid QJsonDocument parser with the downloaded data and
     * returns a future to the parser.
     * \param uri URI to download
     *
     * \return Future of valid parser filled with the data downloaded
     */
    JsonDocumentFuture async_get_parser(std::string const& uri) const;

    /**
     * \brief Downloads a HTTP remote file asynchronously and returns a future to a valid JSON parser containing the
     *data.
     *
     * This method downloads a remote http document, fills a valid QJsonDocument parser with the downloaded data and
     * returns a future to the parser.
     * \param host host name
     * \param params The parameters that will build the final query, defined by a list of pairs of key and value
     *
     * \return Future of valid parser filled with the data downloaded
     */
    JsonDocumentFuture async_get_parser(std::string const& host, JsonParameters const& params) const;

protected:
    /**
     * \brief Creates a QJsonDocument filled with the given data.
     * \param data The data that contains the JSON document
     * \param error Returned by the method, contains any possible error string obtained during the creation of the
     *parser.
     *
     * \return shared pointer to the QJsonDocument created
     */
    static QJsonDocumentSptr create_parser_with_data(std::string const& data, std::string& error);

    /// @cond
    std::shared_ptr<HttpAsyncReader> p_;
    /// @endcond
};

template <typename BASE, typename TYPE>
JsonAsyncReader::ResultsFuture<BASE> JsonAsyncReader::async_get(std::string const& uri,
                                                                std::string const& object_name,
                                                                ParseFunc<BASE> const& parse) const
{
    return p_->async_get<BASE, TYPE, QJsonDocument>(uri, object_name, JsonAsyncReader::create_parser_with_data, parse);
}

template <typename T>
JsonAsyncReader::ResultsFuture<T> JsonAsyncReader::async_get(std::string const& uri,
                                                             std::string const& object_name,
                                                             ParseFunc<T> const& parse) const
{
    return p_->async_get<T, T, QJsonDocument>(uri, object_name, JsonAsyncReader::create_parser_with_data, parse);
}

template <typename BASE, typename TYPE>
JsonAsyncReader::ResultsFuture<BASE> JsonAsyncReader::async_get(std::string const& host,
                                                                JsonParameters const& params,
                                                                std::string const& object_name,
                                                                ParseFunc<BASE> const& parse) const
{
    std::string uri = p_->get_uri(host, params);
    return p_->async_get<BASE, TYPE, QJsonDocument>(uri, object_name, JsonAsyncReader::create_parser_with_data, parse);
}

template <typename T>
JsonAsyncReader::ResultsFuture<T> JsonAsyncReader::async_get(std::string const& host,
                                                             JsonParameters const& params,
                                                             std::string const& object_name,
                                                             ParseFunc<T> const& parse) const
{
    std::string uri = p_->get_uri(host, params);
    return p_->async_get<T, T, QJsonDocument>(uri, object_name, JsonAsyncReader::create_parser_with_data, parse);
}

template <typename B, typename T>
static bool get_results_json_object(QJsonObject& root,
                                    const std::string& object_name,
                                    JsonAsyncReader::ResultsList<B>& results,
                                    std::string& error_string)
{
    QString qobject_name(object_name.c_str());
    if (root.contains(qobject_name))
    {
        if (root[qobject_name].isArray())
        {
            auto sub_root = root[qobject_name];
            QJsonArray levelArray = sub_root.toArray();
            for (int levelIndex = 0; levelIndex < levelArray.size(); ++levelIndex)
            {
                QJsonObject levelObject = levelArray[levelIndex].toObject();
                results.emplace_back(std::make_shared<T>(levelObject));
            }
        }
        else if (root[qobject_name].isObject())
        {
            auto sub_root = root[qobject_name].toObject();
            results.emplace_back(std::make_shared<T>(sub_root));
        }
    }
    else
    {
        // iterate thow the existing members
        for (auto member : root.keys())
        {
            auto item = root[member];
            if (item.isObject())
            {
                QJsonObject obj = item.toObject();
                get_results_json_object<B, T>(obj, object_name, results, error_string);
            }
        }
    }
    return true;
}

template <typename B, typename T>
static bool get_results_json(QJsonDocument& doc,
                             const std::string& object_name,
                             JsonAsyncReader::ResultsList<B>& results,
                             std::string& error_string)
{
    if (doc.isArray())
    {
        QJsonArray levelArray = doc.array();
        for (int levelIndex = 0; levelIndex < levelArray.size(); ++levelIndex)
        {
            QJsonObject levelObject = levelArray[levelIndex].toObject();
            get_results_json_object<B, T>(levelObject, object_name, results, error_string);
        }
    }
    else if (doc.isObject())
    {
        QJsonObject levelObject = doc.object();
        get_results_json_object<B, T>(levelObject, object_name, results, error_string);
    }
    return true;
}

}  // namespace qt

}  // namespace scopes

}  // namespace unity

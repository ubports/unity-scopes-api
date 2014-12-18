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

#include <unity/util/DefinesPtrs.h>
#include <unity/util/NonCopyable.h>
#include <unity/UnityExceptions.h>

#include <unity/scopes/ScopeExceptions.h>

#include <core/net/http/client.h>
#include <core/net/http/request.h>
#include <core/net/http/response.h>

#include <deque>
#include <future>
#include <memory>
#include <vector>

namespace unity
{

namespace scopes
{

namespace qt
{

/**
\brief Class that downloads http files asynchronously.

Executes a remote HTTP query asynchronously to return different futures about results or a parser.
The user should define which parser wants to use, and offer a method that given a char* containing the
data downloaded by this class contructs the parser.
*/
class HttpAsyncReader
{
public:
    /// @cond
    NONCOPYABLE(HttpAsyncReader);
    UNITY_DEFINES_PTRS(HttpAsyncReader);

    // aliases
    template <typename PARSER>
    using FactoryFunc = std::function<std::shared_ptr<PARSER>(const std::string&, std::string&)>;

    template <typename TYPE, typename PARSER>
    using ParseFunc =
        std::function<bool(PARSER& root, const std::string&, std::deque<std::shared_ptr<TYPE>>&, std::string&)>;

    template <typename TYPE>
    using ResultsList = std::deque<std::shared_ptr<TYPE>>;

    template <typename TYPE>
    using ResultsFuture = std::future<ResultsList<TYPE>>;

    template <typename PARSER>
    using ParserFuture = std::future<std::shared_ptr<PARSER>>;

    HttpAsyncReader();

    virtual ~HttpAsyncReader();
    /// @endcond

    /**
     * \brief Downloads a HTTP remote file asynchronously and returns a future to a list of results
     * This method downloads in a separated thread a http document identified by the given URI.
     * It returns a future of list of results based on a given object name.
     * If, for example, the document contains a list of objects identified by the tag "ITEM" it
     * parses the document and returns a list of those objects.
     * The user must provide a function to create the parser for the downloaded data and a parse function
     * that retrieves the data.
     *
     * The method has 3 template parameters: the type of results returned, the type of objects when intantiating
     * and the parser type.
     * The type of instantiation has to be a derived class of the return type. This is offered for convenience
     * to return more generic types and get advantage of polymorphism.
     *
     * The method checks at compile time that the instantiation type is effectively a derived class of the type
     * returned.
     *
     * \param uri URI to download
     * \param object_name name of the kind of object we are looking for in the http document
     * \param create Function that returns a valid parser filled with the data contained in the http document
     * \param parse Function that parses the data downloaded
     *
     * \return Future of list of results
     */
    template <typename BASE, typename TYPE, typename PARSER>
    ResultsFuture<BASE> async_get(std::string const& uri,
                                  std::string const& object_name,
                                  FactoryFunc<PARSER> const& create,
                                  ParseFunc<BASE, PARSER> const& parse) const;

    /**
     * \brief Downloads a HTTP remote file asynchronously and returns a future to a list of results
     * This method downloads in a separated thread a http document identified by the given URI.
     * It returns a future of list of results based on a given object name.
     * If, for example, the document contains a list of objects identified by the tag "ITEM" it
     * parses the document and returns a list of those objects.
     * The user must provide a function to create the parser for the downloaded data and a parse function
     * that retrieves the data.
     *
     * The method has 2 template parameters: the type of objects returned and the parser type.
     *
     * \param uri URI to download
     * \param object_name name of the kind of object we are looking for in the http document
     * \param create Function that returns a valid parser filled with the data contained in the http document
     * \param parse Function that parses the data downloaded
     *
     * \return Future of list of results
     */
    template <typename TYPE, typename PARSER>
    ResultsFuture<TYPE> async_get(std::string const& uri,
                                  std::string const& object_name,
                                  FactoryFunc<PARSER> const& create,
                                  ParseFunc<TYPE, PARSER> const& parse) const;

    /**
     * \brief Downloads a HTTP remote file asynchronously and returns a future to a valid parser containing the data.
     *
     * This method downloads a remote http document, fills a valid parser with the downloaded data and
     * returns a future to the parser.
     * \param uri URI to download
     * \param create Function that returns a valid parser filled with the data contained in the http document
     *
     * \return Future of valid parser filled with the data downloaded
     */
    template <typename PARSER>
    ParserFuture<PARSER> async_get_parser(std::string const& uri, FactoryFunc<PARSER> const& create) const;

    /**
     * \brief Gets the data of the given future in the gived timeout.
     * If the time given expires and the data in the future is not ready throws a unity::scopes::TimeoutException
     *exception
     *
     * \param f Future
     * \param seconds Maximum time to wait for the result
     *
     * \return Result of the given future.
     */
    template <typename T>
    static T get_or_throw(std::future<T>& f, int64_t seconds = 10);

    /**
     * \brief Constructs a URI with the given host and parameters.
     * This is a convenience method that constructs a uri with a given host and parameterss
     */
    std::string get_uri(std::string const& host,
                        std::vector<std::pair<std::string, std::string>> const& parameters) const;

protected:
    /// @cond
    core::net::http::Request::Progress::Next progress_report(core::net::http::Request::Progress const& progress) const;

    void async_execute(core::net::http::Request::Handler const& handler, std::string const& uri) const;

    class Priv;
    std::shared_ptr<Priv> p_;
    /// @endcond
};

template <typename BASE, typename TYPE, typename PARSER>
HttpAsyncReader::ResultsFuture<BASE> HttpAsyncReader::async_get(std::string const& uri,
                                                                std::string const& object_name,
                                                                FactoryFunc<PARSER> const& create,
                                                                ParseFunc<BASE, PARSER> const& parse) const
{
    static_assert(std::is_base_of<BASE, TYPE>::value,
                  "Second template parameter type must be a valid base class of the first one.");

    auto prom = std::make_shared<std::promise<std::deque<std::shared_ptr<BASE>>>>();
    core::net::http::Request::Handler handler;

    handler.on_progress(bind(&HttpAsyncReader::progress_report, this, std::placeholders::_1));

    handler.on_error([prom, uri](core::net::Error const& e)
                     {
                         unity::LogicException logic_exception("AsyncReader::async_get: " + std::string(e.what()) +
                                                               "( uri = " + uri + " )");
                         prom->set_exception(logic_exception.self());
                     });

    handler.on_response(
        [prom, object_name, parse, create, uri](core::net::http::Response const& response)
        {
            if (response.status != core::net::http::Status::ok)
            {
                unity::LogicException e("AsyncReader::async_get: " + response.body + "( uri = " + uri + " )");
                prom->set_exception(e.self());
            }
            else
            {
                std::string error_string;
                std::shared_ptr<PARSER> reader = create(response.body, error_string);
                if (!reader)
                {
                    unity::LogicException e("AsyncReader::async_get: error obtaining parser: " + error_string);
                    prom->set_exception(e.self());
                }
                else
                {
                    std::deque<std::shared_ptr<BASE>> results;
                    std::string error_message;
                    if (!parse(*reader, object_name, results, error_message))
                    {
                        unity::LogicException e("AsyncReader::async_get: error parsing data: " + error_message);
                        prom->set_exception(e.self());
                    }
                    else
                    {
                        prom->set_value(results);
                    }
                }
            }
        });

    async_execute(handler, uri);

    return prom->get_future();
}

template <typename TYPE, typename PARSER>
HttpAsyncReader::ResultsFuture<TYPE> HttpAsyncReader::async_get(std::string const& uri,
                                                                std::string const& object_name,
                                                                FactoryFunc<PARSER> const& create,
                                                                ParseFunc<TYPE, PARSER> const& parse) const
{
    return async_get<TYPE, TYPE, PARSER>(uri, object_name, create, parse);
}

template <typename PARSER>
HttpAsyncReader::ParserFuture<PARSER> HttpAsyncReader::async_get_parser(std::string const& uri,
                                                                        FactoryFunc<PARSER> const& create) const
{
    auto prom = std::make_shared<std::promise<std::shared_ptr<PARSER>>>();
    core::net::http::Request::Handler handler;
    handler.on_progress(bind(&HttpAsyncReader::progress_report, this, std::placeholders::_1));

    handler.on_error([prom, uri](core::net::Error const& e)
                     {
                         unity::LogicException logic_exception("AsyncReader::async_get: " + std::string(e.what()) +
                                                               "( uri = " + uri + " )");
                         prom->set_exception(logic_exception.self());
                     });

    handler.on_response(
        [this, prom, create, uri](core::net::http::Response const& response)
        {
            if (response.status != core::net::http::Status::ok)
            {
                unity::LogicException e("AsyncReader::async_get_parser: " + response.body + "( uri = " + uri + " )");
                prom->set_exception(e.self());
            }
            else
            {
                std::string error_string;
                std::shared_ptr<PARSER> reader = create(response.body, error_string);
                if (!reader)
                {
                    unity::LogicException e("AsyncReader::async_get: error obtaining parser: " + error_string);
                    prom->set_exception(e.self());
                }
                else
                {
                    prom->set_value(reader);
                }
            }
        });

    async_execute(handler, uri);

    return prom->get_future();
}

template <typename T>
T HttpAsyncReader::get_or_throw(std::future<T>& f, int64_t seconds)
{
    if (f.wait_for(std::chrono::seconds(seconds)) != std::future_status::ready)
    {
        throw unity::scopes::TimeoutException("AsyncReader::get_or_throw: Wait for future timeout after " +
                                              std::to_string(seconds) + " seconds");
    }
    return f.get();
}

}  // namespace qt

}  // namespace scopes

}  // namespace unity

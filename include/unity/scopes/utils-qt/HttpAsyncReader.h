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

namespace utils
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
    /// @endcond

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

    template <typename BASE, typename TYPE, typename PARSER>
    ResultsFuture<BASE> async_get(std::string const& uri,
                                  std::string const& object_name,
                                  FactoryFunc<PARSER> const& create,
                                  ParseFunc<BASE, PARSER> const& parse) const;

    template <typename TYPE, typename PARSER>
    ResultsFuture<TYPE> async_get(std::string const& uri,
                                  std::string const& object_name,
                                  FactoryFunc<PARSER> const& create,
                                  ParseFunc<TYPE, PARSER> const& parse) const;

    template <typename PARSER>
    ParserFuture<PARSER> async_get_parser(std::string const& uri, FactoryFunc<PARSER> const& create) const;

    template <typename T>
    static T get_or_throw(std::future<T>& f, int64_t seconds = 10);

    std::string get_uri(std::string const& host,
                        std::vector<std::pair<std::string, std::string>> const& parameters) const;

protected:
    core::net::http::Request::Progress::Next progress_report(core::net::http::Request::Progress const&) const;

    void async_execute(core::net::http::Request::Handler const& handler, std::string const& uri) const;

    class Priv;
    std::shared_ptr<Priv> p_;
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

    handler.on_error([prom](core::net::Error const& e)
                     {
                         prom->set_exception(make_exception_ptr(e));
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

    handler.on_error([prom](core::net::Error const& e)
                     {
                         prom->set_exception(make_exception_ptr(e));
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

}  // namespace utils

}  // namespace scopes

}  // namespace unity

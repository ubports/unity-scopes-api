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
#include <unity/scopes/utils-qt/HttpAsyncReader.h>

#include <core/net/uri.h>

namespace http = core::net::http;
using namespace unity::scopes::utils::qt;

// Private class that holds the httpclient and the worker thread
class HttpAsyncReader::Priv
{
public:
    NONCOPYABLE(Priv);
    UNITY_DEFINES_PTRS(Priv);
    Priv()
        : client_(http::make_client())
        , worker_{[this]()
                  {
                      client_->run();
                  }}
        , cancelled_(false)
    {
    }

    ~Priv()
    {
        client_->stop();
        if (worker_.joinable())
        {
            worker_.join();
        }
    }

    std::shared_ptr<core::net::http::Client> client_;

    std::thread worker_;

    std::atomic<bool> cancelled_;
};

HttpAsyncReader::HttpAsyncReader()
    : p_(new HttpAsyncReader::Priv)
{
}

HttpAsyncReader::~HttpAsyncReader()
{
}

core::net::http::Request::Progress::Next HttpAsyncReader::progress_report(
    core::net::http::Request::Progress const&) const
{
    return p_->cancelled_ ? http::Request::Progress::Next::abort_operation :
                            http::Request::Progress::Next::continue_operation;
}

void HttpAsyncReader::async_execute(core::net::http::Request::Handler const& handler, std::string const& uri) const
{
    http::Request::Configuration configuration;
    configuration.uri = uri;
    configuration.header.add("User-Agent", "test-agent");

    auto request = p_->client_->head(configuration);
    request->async_execute(handler);
}

std::string HttpAsyncReader::get_uri(std::string const& host,
                                     std::vector<std::pair<std::string, std::string>> const& parameters) const
{
    core::net::Uri::Path path;
    core::net::Uri uri = core::net::make_uri(host, path, parameters);
    return p_->client_->uri_to_string(uri);
}

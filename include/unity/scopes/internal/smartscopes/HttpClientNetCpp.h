/*
 * Copyright (C) 2013 Canonical Ltd
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
 * Authored by: Marcus Tomlinson <marcus.tomlinson@canonical.com>
 */

#ifndef UNITY_SCOPES_INTERNAL_SMARTSCOPES_HTTPCLIENTNETCPP_H
#define UNITY_SCOPES_INTERNAL_SMARTSCOPES_HTTPCLIENTNETCPP_H

#include <unity/scopes/internal/smartscopes/HttpClientInterface.h>

#include <memory>
#include <thread>

namespace core
{
namespace net
{
namespace http
{
class StreamingClient;
}
}
}

namespace unity
{

namespace scopes
{

namespace internal
{

namespace smartscopes
{

class HttpClientNetCpp : public HttpClientInterface
{
public:
    explicit HttpClientNetCpp(unsigned int no_reply_timeout);
    ~HttpClientNetCpp();

    HttpResponseHandle::SPtr get(std::string const& request_url,
                                 std::function<void(std::string const&)> const& line_data = [](std::string const&) {},
                                 HttpHeaders const& headers = HttpHeaders()) override;

    std::string to_percent_encoding(std::string const& string) override;

private:
    void cancel_get(unsigned int session_id) override;

    unsigned int no_reply_timeout;
    std::shared_ptr<core::net::http::StreamingClient> client;
    std::thread worker;
};

}  // namespace smartscopes

}  // namespace internal

}  // namespace scopes

}  // namespace unity

#endif  // UNITY_SCOPES_INTERNAL_SMARTSCOPES_HTTPCLIENTNETCPP_H

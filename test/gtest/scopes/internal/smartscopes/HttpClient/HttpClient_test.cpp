/*
 * Copyright © 2013 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License version 3,
 * as published by the Free Software Foundation.
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

#include <unity/scopes/internal/smartscopes/HttpClientNetCpp.h>
#include <unity/UnityExceptions.h>

#include "../RaiiServer.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <gtest/gtest.h>
#pragma GCC diagnostic pop

#include <memory>
#include <thread>

using namespace testing;
using namespace unity::scopes::internal::smartscopes;
using namespace unity::test::scopes::internal::smartscopes;

namespace
{

const std::string c_test_url = "http://127.0.0.1";

class HttpClientTest : public Test
{
public:
    HttpClientTest(unsigned int no_reply_timeout = 20000)
        : http_client_(new HttpClientNetCpp(no_reply_timeout)),
          server_(FAKE_SERVER_PATH)
    {
        test_url_ = c_test_url + ":" + std::to_string(server_.port_);
    }

protected:
    HttpClientInterface::SPtr http_client_;
    RaiiServer server_;
    std::string test_url_;
};

class HttpClientTestQuick : public HttpClientTest
{
public:
    HttpClientTestQuick()
        : HttpClientTest(2000) {}
};

TEST_F(HttpClientTest, no_server)
{
    int dead_port;
    {
        // spawn a server, so it allocates a free port
        RaiiServer server(FAKE_SERVER_PATH);
        dead_port = server.port_;
        // RaiiServer goes out of scope, so it gets killed
    }
    // no server
    HttpResponseHandle::SPtr response = http_client_->get(c_test_url + ":" + std::to_string(dead_port));
    response->wait();

    EXPECT_THROW(response->get(), unity::Exception);
}

TEST_F(HttpClientTest, bad_server)
{
    // bad server
    HttpResponseHandle::SPtr response = http_client_->get(test_url_ + "?x");
    response->wait();

    EXPECT_THROW(response->get(), unity::Exception);
}

TEST_F(HttpClientTest, good_server)
{
    // responds immediately
    std::string response_str;
    HttpResponseHandle::SPtr response = http_client_->get(test_url_, [&response_str](std::string const& s) { response_str += s; });
    response->wait();

    EXPECT_NO_THROW(response->get());
    EXPECT_EQ("Hello there", response_str);
}

TEST_F(HttpClientTestQuick, ok_server)
{
    std::string response_str;

    // responds in 1 second
    HttpResponseHandle::SPtr response = http_client_->get(test_url_ + "?1", [&response_str](std::string const& s) { response_str += s; });
    response->wait();

    EXPECT_EQ("Hello there", response_str);
}

TEST_F(HttpClientTestQuick, slow_server)
{
    // responds in 3 seconds
    std::string response_str;
    HttpResponseHandle::SPtr response = http_client_->get(test_url_ + "?3", [&response_str](std::string const& s) { response_str += s; });
    response->wait();

    EXPECT_THROW(response->get(), unity::Exception);
}

TEST_F(HttpClientTest, multiple_sessions)
{
    std::string response_str1, response_str2, response_str3, response_str4, response_str5;

    HttpResponseHandle::SPtr response1 = http_client_->get(test_url_, [&response_str1](std::string const& s) { response_str1 += s; });
    HttpResponseHandle::SPtr response2 = http_client_->get(test_url_, [&response_str2](std::string const& s) { response_str2 += s; });
    HttpResponseHandle::SPtr response3 = http_client_->get(test_url_, [&response_str3](std::string const& s) { response_str3 += s; });
    HttpResponseHandle::SPtr response4 = http_client_->get(test_url_, [&response_str4](std::string const& s) { response_str4 += s; });
    HttpResponseHandle::SPtr response5 = http_client_->get(test_url_, [&response_str5](std::string const& s) { response_str5 += s; });

    response1->wait();
    response2->wait();
    response3->wait();
    response4->wait();
    response5->wait();

    EXPECT_NO_THROW(response1->get());
    EXPECT_EQ("Hello there", response_str1);
    EXPECT_NO_THROW(response2->get());
    EXPECT_EQ("Hello there", response_str2);
    EXPECT_NO_THROW(response3->get());
    EXPECT_EQ("Hello there", response_str3);
    EXPECT_NO_THROW(response4->get());
    EXPECT_EQ("Hello there", response_str4);
    EXPECT_NO_THROW(response5->get());
    EXPECT_EQ("Hello there", response_str5);
}

TEST_F(HttpClientTest, cancel_get)
{
    HttpResponseHandle::SPtr response = http_client_->get(test_url_ + "?18");
    response->cancel();
    response->wait();

    EXPECT_THROW(response->get(), unity::Exception);
}

TEST_F(HttpClientTest, percent_encoding)
{
    std::string encoded_str = http_client_->to_percent_encoding(" \"%<>\\^`{|}!*'();:@&=+$,/?#[]");
    EXPECT_EQ("%20%22%25%3C%3E%5C%5E%60%7B%7C%7D%21%2A%27%28%29%3B%3A%40%26%3D%2B%24%2C%2F%3F%23%5B%5D", encoded_str);
}

} // namespace

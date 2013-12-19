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

#include <scopes/internal/smartscopes/HttpClientQt.h>
#include <unity/UnityExceptions.h>

#include <gtest/gtest.h>
#include <memory>
#include <thread>

using namespace testing;
using namespace unity::api::scopes::internal::smartscopes;

namespace
{

const std::string test_url = "http://127.0.0.1";

class HttpClientTest : public Test
{
public:
    HttpClientTest(uint no_reply_timeout = 20000)
        : http_client_(new HttpClientQt(2, no_reply_timeout)) {}

    class server_raii
    {
    public:
        server_raii()
        {
            int pipefd[2];
            if (pipe(pipefd) < 0)
            {
                throw unity::ResourceException("Pipe creation failed");
            }

            switch (pid_ = fork())
            {
                case -1:
                    throw unity::ResourceException("Failed to fork process");
                case 0: // child
                    close(STDOUT_FILENO);   // close stdout
                    close(pipefd[0]);       // close read
                    if (dup(pipefd[1]) < 0) // open write
                    {
                        throw unity::ResourceException("Write pipe duplication failed");
                    }

                    execl(FAKE_SERVER_PATH, "", NULL);
                    throw unity::ResourceException("Failed to execute fake server script");
                default: // parent
                    close(STDIN_FILENO);    // close stdin
                    close(pipefd[1]);       // close write
                    if (dup(pipefd[0]) < 0) // open read
                    {
                        throw unity::ResourceException("Read pipe duplication failed");
                    }

                    char port_str[6];
                    read(pipefd[0], port_str, 5);
                    port_ = std::atoi(port_str);
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }

        ~server_raii()
        {
            kill(pid_, SIGABRT);
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }

        pid_t pid_ = -1;
        int port_ = 0;
    };

protected:
    HttpClientInterface::SPtr http_client_;
    server_raii server_;
};

class HttpClientTestQuick : public HttpClientTest
{
public:
    HttpClientTestQuick()
        : HttpClientTest(2000) {}
};

TEST_F(HttpClientTest, no_server)
{
    // no server
    HttpResponseHandle::SPtr response = http_client_->get(test_url, 0);
    response->wait();

    EXPECT_THROW(response->get(), unity::Exception);
}

TEST_F(HttpClientTest, bad_server)
{
    // bad server
    HttpResponseHandle::SPtr response = http_client_->get(test_url + "?x", server_.port_);
    response->wait();

    EXPECT_THROW(response->get(), unity::Exception);
}

TEST_F(HttpClientTest, good_server)
{
    // responds immediately
    HttpResponseHandle::SPtr response = http_client_->get(test_url, server_.port_);
    response->wait();

    std::string response_str;
    EXPECT_NO_THROW(response_str = response->get());
    EXPECT_EQ("Hello there", response_str);
}

TEST_F(HttpClientTestQuick, ok_server)
{
    // responds in 1 second
    HttpResponseHandle::SPtr response = http_client_->get(test_url + "?1", server_.port_);
    response->wait();

    std::string response_str;
    EXPECT_NO_THROW(response_str = response->get());
    EXPECT_EQ("Hello there", response_str);
}

TEST_F(HttpClientTestQuick, slow_server)
{
    // responds in 3 seconds
    HttpResponseHandle::SPtr response = http_client_->get(test_url + "?3", server_.port_);
    response->wait();

    EXPECT_THROW(response->get(), unity::Exception);
}

TEST_F(HttpClientTest, multiple_sessions)
{
    HttpResponseHandle::SPtr response1 = http_client_->get(test_url, server_.port_);
    HttpResponseHandle::SPtr response2 = http_client_->get(test_url, server_.port_);
    HttpResponseHandle::SPtr response3 = http_client_->get(test_url, server_.port_);
    HttpResponseHandle::SPtr response4 = http_client_->get(test_url, server_.port_);
    HttpResponseHandle::SPtr response5 = http_client_->get(test_url, server_.port_);

    response1->wait();
    response2->wait();
    response3->wait();
    response4->wait();
    response5->wait();

    std::string response_str;
    EXPECT_NO_THROW(response_str = response1->get());
    EXPECT_EQ("Hello there", response_str);
    EXPECT_NO_THROW(response_str = response2->get());
    EXPECT_EQ("Hello there", response_str);
    EXPECT_NO_THROW(response_str = response3->get());
    EXPECT_EQ("Hello there", response_str);
    EXPECT_NO_THROW(response_str = response4->get());
    EXPECT_EQ("Hello there", response_str);
    EXPECT_NO_THROW(response_str = response5->get());
    EXPECT_EQ("Hello there", response_str);
}

TEST_F(HttpClientTest, cancel_get)
{
    HttpResponseHandle::SPtr response = http_client_->get(test_url + "?18", server_.port_);
    http_client_->cancel_get(response);
    response->wait();

    EXPECT_THROW(response->get(), unity::Exception);
}

TEST_F(HttpClientTest, percent_encoding)
{
    std::string encoded_str = http_client_->to_percent_encoding(" \"%<>\\^`{|}!*'();:@&=+$,/?#[]");
    EXPECT_EQ("%20%22%25%3C%3E%5C%5E%60%7B%7C%7D%21%2A%27%28%29%3B%3A%40%26%3D%2B%24%2C%2F%3F%23%5B%5D", encoded_str);
}

} // namespace

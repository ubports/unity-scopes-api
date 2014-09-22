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

#ifndef UNITY_SCOPES_INTERNAL_SMARTSCOPES_HTTPCLIENTQT_H
#define UNITY_SCOPES_INTERNAL_SMARTSCOPES_HTTPCLIENTQT_H

#include <unity/scopes/internal/smartscopes/HttpClientInterface.h>
#include <map>

class QCoreApplication;

namespace unity
{

namespace scopes
{

namespace internal
{

namespace smartscopes
{

class HttpClientQtThread;

class HttpClientQt : public HttpClientInterface
{
public:
    explicit HttpClientQt(unsigned int no_reply_timeout);
    ~HttpClientQt();

    HttpResponseHandle::SPtr get(std::string const& request_url, std::function<void(std::string const&)> const& lineData = [](std::string const&) {},
            HttpHeaders const& headers = HttpHeaders()) override;

    std::string to_percent_encoding(std::string const& string) override;

private:
    void cancel_get(unsigned int session_id) override;

private:
    class HttpSession
    {
    public:
        HttpSession(std::string const& request_url, unsigned int timeout, std::function<void(std::string const&)> const& lineData, HttpHeaders const& headers =
                HttpHeaders());
        ~HttpSession();

        std::future<void> get_future();

        void cancel_session();
        void wait_for_session();

    private:
        std::promise<void> promise_;
        std::thread get_thread_;
        std::unique_ptr<HttpClientQtThread> qt_thread_;
        std::mutex qt_thread_mutex_;
        std::promise<void> qt_thread_ready_;
    };

private:
    unsigned int session_index_;
    std::map<unsigned int, std::shared_ptr<HttpSession>> sessions_;
    std::mutex sessions_mutex_;

    unsigned int const no_reply_timeout_;

    std::unique_ptr<QCoreApplication> app_;
};

}  // namespace smartscopes

}  // namespace internal

}  // namespace scopes

}  // namespace unity

#endif  // UNITY_SCOPES_INTERNAL_SMARTSCOPES_HTTPCLIENTQT_H

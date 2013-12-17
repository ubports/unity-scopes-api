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

#ifndef UNITY_API_SCOPES_INTERNAL_SMARTSCOPES_HTTPCLIENTQT_H
#define UNITY_API_SCOPES_INTERNAL_SMARTSCOPES_HTTPCLIENTQT_H

#include <scopes/internal/smartscopes/HttpClientInterface.h>
#include <map>

class QCoreApplication;
class HttpClientQtThread;

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

namespace smartscopes
{

class HttpClientQt : public HttpClientInterface
{
public:
    HttpClientQt(uint max_sessions, uint no_reply_timeout = 2000);
    ~HttpClientQt();

    HttpSessionHandle::SPtr get(std::string const& request_url, int port = 80) override;
    void cancel_get(const HttpSessionHandle::SPtr& session_handle) override;

    std::string to_percent_encoding(std::string const& string) override;

private:
    class HttpSession
    {
    public:
        HttpSession(std::string const& request_url, int port, uint timeout);
        ~HttpSession();

        std::future<std::string> get_future();

        void cancel_session();
        void wait_for_session();

    private:
        std::shared_ptr<std::promise<std::string>> promise_;
        std::thread get_thread_;
        std::unique_ptr<HttpClientQtThread> get_qt_thread_;
    };

private:
    uint session_index_;
    std::map<uint, std::shared_ptr<HttpSession>> sessions_;
    uint max_sessions_;
    uint no_reply_timeout_;

    std::unique_ptr<QCoreApplication> app_;
};

} // namespace smartscopes

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

#endif // UNITY_API_SCOPES_INTERNAL_SMARTSCOPES_HTTPCLIENTQT_H

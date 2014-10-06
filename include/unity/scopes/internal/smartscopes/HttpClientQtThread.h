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

#ifndef UNITY_SCOPES_INTERNAL_SMARTSCOPES_HTTPCLIENTQTTHREAD_H
#define UNITY_SCOPES_INTERNAL_SMARTSCOPES_HTTPCLIENTQTTHREAD_H

// This hack allows unity-scopes-api to be built with
// clang-3.4+ and versions of Qt before v5.1.1.
#if QT_VERSION < 0x050101
    #define register

    #include <QThread>

    #define qHash(x,y) qHash(const QUrl &url, unsigned int seed)
    #include <QUrl>
    #undef qHash

    #undef register
#else
    #include <QThread>
    #include <QUrl>
#endif

#include <unity/scopes/internal/smartscopes/HttpClientInterface.h>
#include <unity/util/NonCopyable.h>

#include <mutex>

class QNetworkReply;
class QNetworkAccessManager;

namespace unity
{

namespace scopes
{

namespace internal
{

namespace smartscopes
{

class Q_DECL_EXPORT HttpClientQtThread : public QThread
{
    Q_OBJECT

public:
    NONCOPYABLE(HttpClientQtThread);

    HttpClientQtThread(const QUrl& url, unsigned int timeout, std::function<void(std::string const&)> const& lineData, const HttpHeaders& headers =
            HttpHeaders());
    ~HttpClientQtThread();

    bool get_reply(std::string& reply);

private:
    void run();

public Q_SLOTS:
    void cancel();
    void timeout();
    void got_reply(QNetworkReply* reply);
    void dataReady();

Q_SIGNALS:
    void abort();

private:
    QUrl url_;
    const std::function<void(std::string const&)> lineDataCallback_;
    HttpHeaders headers_;
    unsigned int timeout_;
    std::mutex reply_mutex_;

    bool success_;
    std::string reply_;
};

} // namespace smartscopes

} // namespace internal

} // namespace scopes

} // namespace unity

#endif  // UNITY_SCOPES_INTERNAL_SMARTSCOPES_HTTPCLIENTQTTHREAD_H

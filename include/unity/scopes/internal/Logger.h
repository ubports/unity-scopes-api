/*
 * Copyright (C) 2014 Canonical Ltd
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
 * Authored by: Michi Henning <michi.henning@canonical.com>
 */

#pragma once

#include <unity/util/DefinesPtrs.h>
#include <unity/util/ResourcePtr.h>

#define BOOST_LOG_DYN_LINK

#include <boost/log/sinks.hpp>
#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/severity_channel_logger.hpp>
#include <boost/log/utility/value_ref.hpp>

#include <atomic>
#include <unordered_map>

namespace unity
{

namespace scopes
{

namespace internal
{

namespace
{

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

BOOST_LOG_ATTRIBUTE_KEYWORD(severity, "Severity", int)
BOOST_LOG_ATTRIBUTE_KEYWORD(channel, "Channel", std::string)

#pragma GCC diagnostic pop

}

class Logger
{
public:
    UNITY_DEFINES_PTRS(Logger);

    // Instantiate a logger that logs to std::clog.
    Logger(std::string const& id);
    ~Logger();

    enum Channel
    {
        IPC,
        LastChannelEnum_
    };

    // Returns default logger (no channel)
    operator boost::log::sources::severity_channel_logger_mt<>&();

    // Returns logger for specified channel.
    boost::log::sources::severity_channel_logger_mt<>& operator()(Channel c);

    void set_log_file(std::string const& path, int rotation_size, int dir_size);

    enum Severity { Info, Warning, Error, Fatal, Trace };
    Severity set_severity_threshold(Severity s);

    bool set_channel(Channel c, bool enable);

private:
    bool filter(boost::log::value_ref<int, tag::severity> const& level,
                boost::log::value_ref<std::string, tag::channel> const& channel);

    void formatter(boost::log::record_view const& rec,
                   boost::log::formatting_ostream& strm);

    std::string scope_id_;  // immutable

    boost::log::sources::severity_channel_logger_mt<> logger_;  // Default logger, no channel, immutable

    std::atomic<Severity> severity_;

    typedef boost::log::sinks::asynchronous_sink<boost::log::sinks::text_ostream_backend> ClogSinkT;
    typedef boost::shared_ptr<ClogSinkT> ClogSinkPtr;
    ClogSinkPtr clog_sink_;

    typedef boost::log::sinks::asynchronous_sink<boost::log::sinks::text_file_backend> FileSinkT;
    typedef boost::shared_ptr<FileSinkT> FileSinkPtr;
    FileSinkPtr file_sink_;

    typedef std::pair<boost::log::sources::severity_channel_logger_mt<>, std::atomic_bool> ChannelData;
    typedef std::unordered_map<std::string, ChannelData> ChannelMap;
    ChannelMap channel_loggers_;  // immutable

    std::mutex mutex_;  // Protects clog_sink_ and file_sink_
};

} // namespace internal

} // namespace scopes

} // namespace unity

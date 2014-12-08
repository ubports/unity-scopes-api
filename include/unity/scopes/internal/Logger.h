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

#include <atomic>
#include <unordered_map>

namespace unity
{

namespace scopes
{

namespace internal
{

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

    void set_log_file(std::string const& path);

    enum Severity { Trace, Info, Warning, Error, Fatal };
    Severity set_severity_threshold(Severity s);

    bool set_channel(Channel c, bool enabled);

private:
    void formatter(boost::log::record_view const& rec,
                   boost::log::formatting_ostream& strm);

    std::string scope_id_;

    boost::log::sources::severity_channel_logger_mt<> logger_;  // Default logger, no channel

    Severity severity_;

    typedef boost::log::sinks::synchronous_sink<boost::log::sinks::text_ostream_backend> ClogSinkT;
    typedef boost::shared_ptr<ClogSinkT> ClogSinkPtr;
    ClogSinkPtr clog_sink_;

    typedef boost::log::sinks::synchronous_sink<boost::log::sinks::text_file_backend> FileSinkT;
    typedef boost::shared_ptr<FileSinkT> FileSinkPtr;
    FileSinkPtr file_sink_;

    typedef std::pair<boost::log::sources::severity_channel_logger_mt<>, std::atomic_bool> ChannelData;
    typedef std::unordered_map<std::string, ChannelData> ChannelMap;
    ChannelMap channel_loggers_;
};

} // namespace internal

} // namespace scopes

} // namespace unity

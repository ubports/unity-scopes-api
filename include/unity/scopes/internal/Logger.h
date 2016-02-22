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
#include <unity/util/NonCopyable.h>

#include <array>
#include <atomic>
#include <iostream>
#include <sstream>

namespace unity
{

namespace scopes
{

namespace internal
{

enum class LoggerSeverity { Info, Warning, Error, Fatal, Trace };

enum class LoggerChannel { DefaultChannel, IPC, LastChannelEnum_ };

class Logger;

class LogStream : public std::ostringstream
{
public:
    NONCOPYABLE(LogStream);
    UNITY_DEFINES_PTRS(LogStream);

#if __GNUC__ == 4
    // gcc 4.9 doesn't have a move constructor for ostringstream:
    // https://gcc.gnu.org/bugzilla/show_bug.cgi?id=54316
    // We simulate the move with a copy.
    LogStream(LogStream&& other)
        : outstream_(other.outstream_)
    {
        *this << other.rdbuf();
        other.str("");
        other.clear();
    }
#else
    // We need an explicit move constructor because basic_ios does not have a move
    // constructor. Instead, it has a protected move() method that ostringstream calls
    // as part if its move constructor.
    LogStream(LogStream&& other)
        : std::ostringstream(std::move(other))
        , outstream_(other.outstream_)
    {
    }
#endif

    LogStream& operator=(LogStream&&) = delete;  // Move assignment is impossible due to reference member.

    LogStream();
    LogStream(std::ostream& outstream, std::string const& id, LoggerSeverity s, LoggerChannel c);
    ~LogStream();

private:
    std::string const id_;
    std::ostream& outstream_;
    LoggerSeverity severity_;
    LoggerChannel channel_;
};

class Logger
{
public:
    NONCOPYABLE(Logger);
    UNITY_DEFINES_PTRS(Logger);

    Logger(Logger&&) = default;
    Logger& operator=(Logger&&) = delete;  // Move assignment is impossible due to reference member.

    // Instantiate a logger that logs to the given stream.
    Logger(std::string const& id, std::ostream& outstream = std::clog);

    // Returns default writer for severity Error on the default channel.
    LogStream operator()();

    // Returns writer for specified severity.
    LogStream operator()(LoggerSeverity s);

    // Returns writer for specified channel.
    LogStream operator()(LoggerChannel c);

    LoggerSeverity set_severity_threshold(LoggerSeverity s);

    bool set_channel(LoggerChannel c, bool enable);
    bool set_channel(std::string channel_name, bool enable);

private:
    std::string const id_;
    std::ostream& outstream_;
    std::atomic<LoggerSeverity> severity_threshold_;
    std::array<std::atomic_bool, int(LoggerChannel::LastChannelEnum_)> enabled_;
};

} // namespace internal

} // namespace scopes

} // namespace unity

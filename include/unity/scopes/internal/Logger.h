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
#include <memory>
#include <sstream>
#include <vector>

namespace unity
{

namespace scopes
{

namespace internal
{

enum class LoggerSeverity { Info, Warning, Error, Fatal, Trace };

enum class LoggerChannel { DefaultChannel, IPC, LastChannelEnum_ };

class Logger;

class LogStreamWriter final
{
public:
    NONCOPYABLE(LogStreamWriter);
    UNITY_DEFINES_PTRS(LogStreamWriter);

    LogStreamWriter(LogStreamWriter&&) = default;
    LogStreamWriter& operator=(LogStreamWriter&&) = default;

    LogStreamWriter();
    LogStreamWriter(std::ostream& log_stream, std::string const& id, LoggerSeverity s, LoggerChannel c);
    ~LogStreamWriter();

    std::ostream& stream();

private:
    std::string const& id_;
    std::ostream& log_stream_;
    LoggerSeverity severity_;
    LoggerChannel channel_;
    std::unique_ptr<std::ostream> buf_;
};

LogStreamWriter operator<<(LogStreamWriter&&, bool);
LogStreamWriter operator<<(LogStreamWriter&&, short);
LogStreamWriter operator<<(LogStreamWriter&&, unsigned short);
LogStreamWriter operator<<(LogStreamWriter&&, int);
LogStreamWriter operator<<(LogStreamWriter&&, unsigned int);
LogStreamWriter operator<<(LogStreamWriter&&, long);
LogStreamWriter operator<<(LogStreamWriter&&, unsigned long);
LogStreamWriter operator<<(LogStreamWriter&&, long long);
LogStreamWriter operator<<(LogStreamWriter&&, unsigned long long);
LogStreamWriter operator<<(LogStreamWriter&&, float);
LogStreamWriter operator<<(LogStreamWriter&&, double);
LogStreamWriter operator<<(LogStreamWriter&&, long double);
LogStreamWriter operator<<(LogStreamWriter&&, char);
LogStreamWriter operator<<(LogStreamWriter&&, signed char);
LogStreamWriter operator<<(LogStreamWriter&&, unsigned char);
LogStreamWriter operator<<(LogStreamWriter&&, char const*);
LogStreamWriter operator<<(LogStreamWriter&&, signed char const*);
LogStreamWriter operator<<(LogStreamWriter&&, unsigned char const*);
LogStreamWriter operator<<(LogStreamWriter&&, std::string const&);
LogStreamWriter operator<<(LogStreamWriter&&, void const*);
LogStreamWriter operator<<(LogStreamWriter&&, std::ostream& (*)(std::ostream&));
LogStreamWriter operator<<(LogStreamWriter&&, std::ios& (*)(std::ios&));
LogStreamWriter operator<<(LogStreamWriter&&, std::ios_base& (*)(std::ios_base&));

class Logger
{
public:
    NONCOPYABLE(Logger);
    UNITY_DEFINES_PTRS(Logger);

    Logger(Logger&&) = default;
    Logger& operator=(Logger&) = default;

    // Instantiate a logger that logs to the given stream.
    Logger(std::string const& id, std::ostream& outstream = std::clog);
    ~Logger();

    // Returns default writer
    operator LogStreamWriter();

    // Returns writer for specified severity.
    LogStreamWriter operator()(LoggerSeverity s);

    // Returns writer for specified channel.
    LogStreamWriter operator()(LoggerChannel c);

    LoggerSeverity set_severity_threshold(LoggerSeverity s);

    bool set_channel(LoggerChannel c, bool enable);
    bool set_channel(std::string channel_name, bool enable);
    void enable_channels(std::vector<std::string> const& names);

private:
    std::string const scope_id_;
    std::ostream& log_stream_;
    std::atomic<LoggerSeverity> severity_threshold_;
    std::array<std::atomic_bool, int(LoggerChannel::LastChannelEnum_)> enabled_;
};

} // namespace internal

} // namespace scopes

} // namespace unity

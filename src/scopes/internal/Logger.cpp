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

#include <unity/scopes/internal/Logger.h>

#include <unity/UnityExceptions.h>

#include <cassert>
#include <chrono>
#include <iomanip>

using namespace std;

namespace unity
{

namespace scopes
{

namespace internal
{

namespace
{

static array<string, 5> const severities = {{"INFO", "WARNING", "ERROR", "FATAL", "TRACE"}};

static array<pair<string, LoggerChannel>, int(LoggerChannel::LastChannelEnum_)> const channel_names =
{
    pair<string, LoggerChannel>{"", LoggerChannel::DefaultChannel},
    pair<string, LoggerChannel>{"IPC", LoggerChannel::IPC}
};

}  // namespace

// Instantiate a logger for the scope/client with the given ID.

Logger::Logger(string const& id, ostream& outstream)
    : id_(id)
    , outstream_(outstream)
    , severity_threshold_(LoggerSeverity::Info)  // By default, log all severities
{
    // Channels are disabled by default.
    for (auto& e : enabled_)
    {
        e = false;
    }
    // Except for the default channel.
    enabled_[0] = true;
}

// Default writes to the default channel at severity Error.

LogStream Logger::operator()()
{
    if (LoggerSeverity::Error >= severity_threshold_)
    {
        return LogStream(outstream_, id_, LoggerSeverity::Error, LoggerChannel::DefaultChannel);
    }
    return LogStream();  // Null writer
}

LogStream Logger::operator()(LoggerSeverity s)
{
    if (s >= severity_threshold_)
    {
        return LogStream(outstream_, id_, s, LoggerChannel::DefaultChannel);
    }
    return LogStream();  // Null writer
}

LogStream Logger::operator()(LoggerChannel c)
{
    if (enabled_[int(c)])
    {
        return LogStream(outstream_, id_, LoggerSeverity::Trace, c);
    }
    return LogStream();  // Null writer
}

bool Logger::set_channel(LoggerChannel c, bool enable)
{
    return enabled_[int(c)].exchange(enable);
}

bool Logger::set_channel(string channel_name, bool enable)
{
    for (auto const& p : channel_names)
    {
        if (!p.first.empty() && channel_name == p.first)
        {
            return set_channel(p.second, enable);
        }
    }
    throw InvalidArgumentException("Logger::set_channel(): invalid channel name: \"" + channel_name + "\"");
}

LoggerSeverity Logger::set_severity_threshold(LoggerSeverity s)
{
    return severity_threshold_.exchange(s);
}

// Null streambuf implementation that always reports success.
// This allows the operator<<() functions to inject into a stream without
// having to check in each function whether we actually want to log something.

template <class cT, class traits = std::char_traits<cT>>
class NullBuf : public std::basic_streambuf<cT, traits>
{
    typename traits::int_type overflow(typename traits::int_type c)
    {
        return traits::not_eof(c);  // Indicate success
    }
};

static string null_id;
static NullBuf<char> null_buf;
static ostream null_stream(&null_buf);

LogStream::LogStream()  // Doesn't log anything
    : id_(null_id)
    , outstream_(null_stream)
    , severity_(static_cast<LoggerSeverity>(0))
    , channel_(static_cast<LoggerChannel>(0))
{
}

LogStream::LogStream(ostream& outstream, string const& id, LoggerSeverity s, LoggerChannel c)
    : id_(id)
    , outstream_(outstream)
    , severity_(s)
    , channel_(c)
{
}

namespace
{

string get_time()
{
    auto now = chrono::system_clock::now();
    auto curr_t = chrono::system_clock::to_time_t(now);
    auto millisecs = chrono::duration_cast<chrono::milliseconds>(now.time_since_epoch()).count() % 1000;

    struct tm result;
    localtime_r(&curr_t, &result);

    // Should use put_time(&result, "%F %T') here, but gcc 4.9 doesn't provide it.
    char buf[]{"yyyy-mm-dd hh:mm:ss"};
    strftime(buf, sizeof(buf), "%F %T", &result);
    stringstream s;
    s << buf << "." << setw(3) << setfill('0') << millisecs;
    return s.str();
}

}  // namespace

LogStream::~LogStream()
{
    string msg = str();
    if (msg.empty())
    {
        return;
    }

    // Something was logged. Accumulate all the details in an output string.
    string prefix = channel_ != LoggerChannel::DefaultChannel
                        ? channel_names[int(channel_)].first
                        : severities[int(severity_)];
    string output = "[" + get_time() + "] " + prefix + ": " + id_ + ": " + msg + "\n";

    // Write the output string with a single inserter, so if different threads
    // log concurrently, we don't get interleaved output.
    outstream_ << output;
}

}  // namespace internal

}  // namespace scopes

}  // namespace unity

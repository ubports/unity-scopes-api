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

Logger::Logger(string const& scope_id, ostream& log_stream)
    : scope_id_(scope_id)
    , log_stream_(log_stream)
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

Logger::~Logger()
{
}

// Default writer writes to the default channel at severity Error.

Logger::operator LogStreamWriter()
{
    if (LoggerSeverity::Error >= severity_threshold_)
    {
        return LogStreamWriter(log_stream_, scope_id_, LoggerSeverity::Error, LoggerChannel::DefaultChannel);
    }
    return LogStreamWriter();  // Null writer
}

LogStreamWriter Logger::operator()(LoggerSeverity s)
{
    if (s >= severity_threshold_)
    {
        return LogStreamWriter(log_stream_, scope_id_, s, LoggerChannel::DefaultChannel);
    }
    return LogStreamWriter();  // Null writer
}

LogStreamWriter Logger::operator()(LoggerChannel c)
{
    if (enabled_[int(c)])
    {
        return LogStreamWriter(log_stream_, scope_id_, LoggerSeverity::Trace, c);
    }
    return LogStreamWriter();  // Null writer
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

void Logger::enable_channels(vector<string> const& names)
{
    exception_ptr ep;
    for (auto&& name : names)
    {
        try
        {
            set_channel(name, true);
        }
        catch (InvalidArgumentException& e)
        {
            ep = ep ? make_exception_ptr(current_exception()) : e.remember(ep);
        }
    }
    if (ep)
    {
        rethrow_exception(ep);
    }
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

LogStreamWriter::LogStreamWriter()  // Doesn't log anything
    : id_(null_id)
    , log_stream_(null_stream)
    , severity_(static_cast<LoggerSeverity>(0))
    , channel_(static_cast<LoggerChannel>(0))
    , buf_(new ostream(&null_buf))
{
}

LogStreamWriter::LogStreamWriter(ostream& log_stream, string const& id, LoggerSeverity s, LoggerChannel c)
    : id_(id)
    , log_stream_(log_stream)
    , severity_(s)
    , channel_(c)
    , buf_(new ostringstream)
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

    stringstream s;
    s << put_time(&result, "%F %T.") << setw(3) << setfill('0') << millisecs;
    return s.str();
}

}  // namespace

LogStreamWriter::~LogStreamWriter()
{
    string msg;

    auto stream = dynamic_cast<ostringstream*>(buf_.get());
    if (!stream || (msg = stream->str()).empty())
    {
        return;
    }

    // Something was logged. Accumulate all the details in a string.
    string time = get_time();
    string prefix = channel_ != LoggerChannel::DefaultChannel
                        ? channel_names[int(channel_)].first
                        : severities[int(severity_)];
    string output = "[" + time + "] " + prefix + ": " + id_ + ": " + msg + "\n";

    // Write the string with a single inserter, so if different threads
    // log concurrently, we don't get interleaved output.
    log_stream_ << output;
}

ostream& LogStreamWriter::stream()
{
    return *buf_;
}

LogStreamWriter operator<<(LogStreamWriter&& w, bool v)
{
    w.stream() << v;
    return move(w);
}

LogStreamWriter operator<<(LogStreamWriter&& w, short v)
{
    w.stream() << v;
    return move(w);
}

LogStreamWriter operator<<(LogStreamWriter&& w, unsigned short v)
{
    w.stream() << v;
    return move(w);
}

LogStreamWriter operator<<(LogStreamWriter&& w, int v)
{
    w.stream() << v;
    return move(w);
}

LogStreamWriter operator<<(LogStreamWriter&& w, unsigned int v)
{
    w.stream() << v;
    return move(w);
}

LogStreamWriter operator<<(LogStreamWriter&& w, long v)
{
    w.stream() << v;
    return move(w);
}

LogStreamWriter operator<<(LogStreamWriter&& w, unsigned long v)
{
    w.stream() << v;
    return move(w);
}

LogStreamWriter operator<<(LogStreamWriter&& w, long long v)
{
    w.stream() << v;
    return move(w);
}

LogStreamWriter operator<<(LogStreamWriter&& w, unsigned long long v)
{
    w.stream() << v;
    return move(w);
}

LogStreamWriter operator<<(LogStreamWriter&& w, float v)
{
    w.stream() << v;
    return move(w);
}

LogStreamWriter operator<<(LogStreamWriter&& w, double v)
{
    w.stream() << v;
    return move(w);
}

LogStreamWriter operator<<(LogStreamWriter&& w, long double v)
{
    w.stream() << v;
    return move(w);
}

LogStreamWriter operator<<(LogStreamWriter&& w, char v)
{
    w.stream() << v;
    return move(w);
}

LogStreamWriter operator<<(LogStreamWriter&& w, signed char v)
{
    w.stream() << v;
    return move(w);
}

LogStreamWriter operator<<(LogStreamWriter&& w, unsigned char v)
{
    w.stream() << v;
    return move(w);
}

LogStreamWriter operator<<(LogStreamWriter&& w, char const* v)
{
    w.stream() << v;
    return move(w);
}

LogStreamWriter operator<<(LogStreamWriter&& w, signed char const* v)
{
    w.stream() << v;
    return move(w);
}

LogStreamWriter operator<<(LogStreamWriter&& w, unsigned char const* v)
{
    w.stream() << v;
    return move(w);
}

LogStreamWriter operator<<(LogStreamWriter&& w, string const& v)
{
    w.stream() << v;
    return move(w);
}

LogStreamWriter operator<<(LogStreamWriter&& w, void const* v)
{
    w.stream() << v;
    return move(w);
}

LogStreamWriter operator<<(LogStreamWriter&& w, ostream& (*v)(ostream&))
{
    v(w.stream());
    return move(w);
}

LogStreamWriter operator<<(LogStreamWriter&& w, ios& (*v)(ios&))
{
    v(w.stream());
    return move(w);
}

LogStreamWriter operator<<(LogStreamWriter&& w, ios_base& (*v)(ios_base&))
{
    v(w.stream());
    return move(w);
}

}  // namespace internal

}  // namespace scopes

}  // namespace unity

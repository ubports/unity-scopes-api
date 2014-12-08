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

#include <boost/log/attributes.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/trivial.hpp>
#include <boost/utility/empty_deleter.hpp>

using namespace std;

namespace logging = boost::log;
namespace sinks = boost::log::sinks;
namespace src = boost::log::sources;
namespace attrs = boost::log::attributes;
namespace expr = boost::log::expressions;
namespace keywords = boost::log::keywords;

namespace unity
{

namespace scopes
{

namespace internal
{

namespace
{

static array<string, Logger::LastChannelEnum_> const channel_names =
    {
        "IPC"
    };

string const& severity(int s)
{
    static array<string, 5> const severities = { { "TRACE", "INFO", "WARNING", "ERROR", "FATAL" } };
    static string const unknown = "UNKNOWN";

    if (s < 0 || s >= static_cast<int>(severities.size()))
    {
        return unknown;
    }
    return severities[s];
}

}

// Instantiate a logger for the scope/client with the given ID.

Logger::Logger(string const& scope_id)
    : scope_id_(scope_id)
    , logger_()
    , severity_(Logger::Info)
{
    namespace ph = std::placeholders;

    assert(!scope_id_.empty());

    logger_.add_attribute("TimeStamp", attrs::local_clock());

    // Create a channel logger for each channel.
    for (auto&& name : channel_names)
    {
        auto clogger = boost::log::sources::severity_channel_logger_mt<>(keywords::channel = name);
        clogger.add_attribute("TimeStamp", attrs::local_clock());
        channel_loggers_[name] = make_pair(clogger, false);
    }

    // Set up sink that logs to std::clog.
    clog_sink_ = boost::make_shared<ClogSinkT>();
    clog_sink_->set_formatter(bind(&Logger::formatter, this, ph::_1, ph::_2));
    boost::shared_ptr<std::ostream> console_stream(&std::clog, boost::empty_deleter());
    clog_sink_->locked_backend()->add_stream(console_stream);
    logging::core::get()->add_sink(clog_sink_);

    set_severity_threshold(severity_);
}

Logger::~Logger()
{
    if (clog_sink_)
    {
        logging::core::get()->remove_sink(clog_sink_);
    }
    else
    {
        logging::core::get()->remove_sink(file_sink_);
    }
}

Logger::operator src::severity_channel_logger_mt<>&()
{
    return logger_;
}

src::severity_channel_logger_mt<>& Logger::operator()(Channel c)
{
    return channel_loggers_[channel_names[c]].first;
}

bool Logger::set_channel(Channel c, bool enabled)
{
    auto it = channel_loggers_.find(channel_names[c]);
    assert(it != channel_loggers_.end());
    bool old_setting = it->second.second.exchange(enabled);
    return old_setting;
}

void Logger::set_log_file(string const& path)
{
    namespace ph = std::placeholders;

    FileSinkPtr s = boost::make_shared<FileSinkT>(
                        keywords::file_name = path + "_%N.log",
                        keywords::rotation_size = 5 * 1024 * 1024,
                        keywords::time_based_rotation = sinks::file::rotation_at_time_point(12, 0, 0));
    if (clog_sink_)
    {
        logging::core::get()->remove_sink(clog_sink_);
    }
    else
    {
        logging::core::get()->remove_sink(file_sink_);
    }
    file_sink_ = s;
    set_severity_threshold(severity_);
    file_sink_->set_formatter(bind(&Logger::formatter, this, ph::_1, ph::_2));
    logging::core::get()->add_sink(s);
}

Logger::Severity Logger::set_severity_threshold(Logger::Severity s)
{
    auto old_s = severity_;
    severity_ = s;
    auto filter = expr::attr<int>("Severity") >= static_cast<int>(severity_);
    if (clog_sink_)
    {
        clog_sink_->set_filter(filter);
    }
    else
    {
        file_sink_->set_filter(filter);
    }
    return old_s;
}

void Logger::formatter(logging::record_view const& rec,
                       logging::formatting_ostream& strm)
{
    string channel = expr::attr<string>("Channel")(rec).get();

    string channel_str;

    if (!channel.empty())
    {
        auto it = channel_loggers_.find(channel);
        if (it != channel_loggers_.end() && !it->second.second)
        {
            return;  // Channel is disabled
        }
        channel_str = "(" + channel + ") ";
    }

    strm << "[" << expr::format_date_time<boost::posix_time::ptime>("TimeStamp", "%Y-%m-%d %H:%M:%S.%f")(rec) << "] "
         << channel_str
         << severity(expr::attr<int>("Severity")(rec).get()) << ": "
         << scope_id_ << ": "
         << rec[expr::smessage];
}

} // namespace internal

} // namespace scopes

} // namespace uannelity

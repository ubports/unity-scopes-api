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

void formatter(logging::record_view const& rec, logging::formatting_ostream& strm, string const& scope_id)
{
    strm << "[" << expr::format_date_time<boost::posix_time::ptime>("TimeStamp", "%Y-%m-%d %H:%M:%S.%f")(rec) << "] "
         << severity(expr::attr<int>("Severity")(rec).get()) << ": "
         << scope_id << ": "
         << rec[expr::smessage];
}

}

// Instantiate a logger for the scope/client with the given ID.
//
// Note: channel is our own address in memory. This gives
//       us a unique identity. The id passed by the run time
//       is *not* necessarily unique. (It is normally, but
//       some of the tests instantiate more than one run time
//       in a single address space, not always necessarily with
//       a unique scope id. Using a unique ID guarantees
//       that a log message writting by one run time isn't
//       also written a second time by a run time that
//       happens to have the same ID.

Logger::Logger(string const& scope_id)
    : scope_id_(scope_id)
    , logger_(keywords::channel = to_string(reinterpret_cast<ptrdiff_t>(this)))
    , severity_(Logger::Info)
{
    namespace ph = std::placeholders;

    assert(!scope_id_.empty());

    logger_.add_attribute("TimeStamp", attrs::local_clock());

    // Set up sink that logs to std::clog.
    clog_sink_ = boost::make_shared<ClogSinkT>();
    clog_sink_->set_formatter(std::bind(formatter, ph::_1, ph::_2, scope_id));
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
    file_sink_->set_formatter(std::bind(formatter, ph::_1, ph::_2, scope_id_));
    logging::core::get()->add_sink(s);
}

Logger::Severity Logger::set_severity_threshold(Logger::Severity s)
{
    auto old_s = severity_;
    severity_ = s;
    auto filter = expr::attr<string>("Channel") == to_string(reinterpret_cast<ptrdiff_t>(this))
                  && expr::attr<int>("Severity") >= static_cast<int>(severity_);
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

} // namespace internal

} // namespace scopes

} // namespace uannelity

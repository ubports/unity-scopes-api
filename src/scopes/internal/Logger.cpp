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
#include <boost/phoenix/bind.hpp>

#if BOOST_VERSION / 100000 == 1
#if ((BOOST_VERSION / 100) % 1000) < 56
#include <boost/utility/empty_deleter.hpp>
typedef boost::empty_deleter NullDeleter;
#else
#include <boost/core/null_deleter.hpp>
typedef boost::null_deleter NullDeleter;
#endif
#endif

#include <unity/UnityExceptions.h>

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

static array<string, Logger::LastChannelEnum_> const channel_names = {{"IPC"}};

string const& to_severity(int s)
{
    static array<string, 5> const severities = {{"INFO", "WARNING", "ERROR", "FATAL", "TRACE"}};
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
    , logger_(keywords::severity = Logger::Error)
    , severity_(Logger::Info)
{
    namespace ph = std::placeholders;

    assert(!scope_id_.empty());

    logger_.add_attribute("TimeStamp", attrs::local_clock());

    // Create a channel logger for each channel.
    for (auto&& name : channel_names)
    {
        auto clogger = boost::log::sources::severity_channel_logger_mt<>(keywords::severity = Logger::Trace,
                                                                         keywords::channel = name);
        clogger.add_attribute("TimeStamp", attrs::local_clock());
        channel_loggers_[name] = make_pair(clogger, false);
    }

    // Set up sink that logs to std::clog.
    clog_sink_ = boost::make_shared<ClogSinkT>();
    clog_sink_->set_formatter(bind(&Logger::formatter, this, ph::_1, ph::_2));
    boost::shared_ptr<std::ostream> console_stream(&std::clog, NullDeleter());
    clog_sink_->locked_backend()->add_stream(console_stream);
    clog_sink_->locked_backend()->auto_flush(true);
    logging::core::get()->add_sink(clog_sink_);

    set_severity_threshold(severity_);
}

Logger::~Logger()
{
    lock_guard<mutex> lock(mutex_);

    if (clog_sink_)
    {
        logging::core::get()->remove_sink(clog_sink_);
        clog_sink_->stop();
        clog_sink_->flush();
    }
    else
    {
        logging::core::get()->remove_sink(file_sink_);
        file_sink_->stop();
        file_sink_->flush();
    }
}

Logger::operator boost::log::sources::severity_channel_logger_mt<>&()
{
    return logger_;  // No lock needed, immutable
}

src::severity_channel_logger_mt<>& Logger::operator()(Channel c)
{
    // No lock needed: channel_loggers_ is immutable, and the boolean is atomic.
    return channel_loggers_[channel_names[c]].first;
}

bool Logger::set_channel(Channel c, bool enable)
{
    auto it = channel_loggers_.find(channel_names[c]);
    assert(it != channel_loggers_.end());
    bool was_enabled = it->second.second.exchange(enable);
    return was_enabled;
}

bool Logger::set_channel(string channel_name, bool enable)
{
    auto it = channel_loggers_.find(channel_name);
    if (it == channel_loggers_.end())
    {
        throw InvalidArgumentException("Logger::set_channel(): invalid channel name: " + channel_name);
    }
    bool was_enabled = it->second.second.exchange(enable);
    return was_enabled;
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

void Logger::set_log_file(string const& path, int rotation_size, int dir_size)
{
    namespace ph = std::placeholders;

    FileSinkPtr s =
        boost::make_shared<FileSinkT>(keywords::file_name = path + "-%N.log", keywords::rotation_size = rotation_size);

    string parent = boost::filesystem::path(path).parent_path().native();
    s->locked_backend()->set_file_collector(sinks::file::make_collector(
        keywords::target = parent, keywords::max_size = dir_size, keywords::min_free_space = 1024 * 1024 * 5));
    try
    {
        s->locked_backend()->scan_for_files();
    }
    catch (std::exception const& e)
    {
        BOOST_LOG_SEV(logger_, Warning) << "RuntimeImpl::Logger(): log rotation failed (path = " << parent
                                        << "): " << e.what();
        return;
    }
    catch (...)
    {
        BOOST_LOG_SEV(logger_, Warning) << "RuntimeImpl::Logger(): log rotation failed (path = " << parent
                                        << "): unknown exception";
        return;
    }

    {
        lock_guard<mutex> lock(mutex_);

        if (clog_sink_)
        {
            logging::core::get()->remove_sink(clog_sink_);
            clog_sink_ = nullptr;
        }
        else
        {
            logging::core::get()->remove_sink(file_sink_);
        }

        file_sink_ = s;
        file_sink_->locked_backend()->auto_flush(true);
        file_sink_->set_formatter(bind(&Logger::formatter, this, ph::_1, ph::_2));
        logging::core::get()->add_sink(file_sink_);
    }
    set_severity_threshold(severity_);
}

Logger::Severity Logger::set_severity_threshold(Logger::Severity s)
{
    auto old_s = severity_.exchange(s);

    lock_guard<mutex> lock(mutex_);

    auto filter = boost::phoenix::bind(&Logger::filter, this, severity.or_none(), channel.or_none());
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

bool Logger::filter(logging::value_ref<int, tag::severity> const& level,
                    logging::value_ref<string, tag::channel> const& channel)
{
    if (level.get() < static_cast<int>(severity_))  // atomic
    {
        return false;
    }
    if (channel.get().empty())
    {
        return true;
    }
    // No lock needed: channel_loggers_ is immutable, and the boolean is atomic.
    auto it = channel_loggers_.find(channel.get());
    assert(it != channel_loggers_.end());
    return it->second.second;
}

void Logger::formatter(logging::record_view const& rec, logging::formatting_ostream& strm)
{
    string channel = expr::attr<string>("Channel")(rec).get();

    string prefix = channel.empty() ? to_severity(expr::attr<int>("Severity")(rec).get()) : channel;

    strm << "[" << expr::format_date_time<boost::posix_time::ptime>("TimeStamp", "%Y-%m-%d %H:%M:%S.%f")(rec) << "] "
         << prefix << ": " << scope_id_ << ": " << rec[expr::smessage];
}

}  // namespace internal

}  // namespace scopes

}  // namespace unity

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

#ifndef UNITY_SCOPES_INTERNAL_LOGGER_H
#define UNITY_SCOPES_INTERNAL_LOGGER_H

#include <unity/util/DefinesPtrs.h>
#include <unity/util/ResourcePtr.h>

#define BOOST_LOG_DYN_LINK

#include <boost/log/sinks.hpp>
#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/severity_channel_logger.hpp>

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

    operator boost::log::sources::severity_channel_logger_mt<>&();

    void set_log_file(std::string const& path);

    enum Severity { Trace, Info, Warning, Error, Fatal };
    Severity set_severity_threshold(Severity s);

private:
    std::string scope_id_;
    boost::log::sources::severity_channel_logger_mt<> logger_;

    typedef boost::log::sinks::synchronous_sink<boost::log::sinks::text_ostream_backend> ClogSinkT;
    typedef boost::shared_ptr<ClogSinkT> ClogSinkPtr;
    ClogSinkPtr clog_sink_;

    typedef boost::log::sinks::synchronous_sink<boost::log::sinks::text_file_backend> FileSinkT;
    typedef boost::shared_ptr<FileSinkT> FileSinkPtr;
    FileSinkPtr file_sink_;

    Severity severity_;
};

} // namespace internal

} // namespace scopes

} // namespace unity

#endif

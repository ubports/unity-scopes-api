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

#ifndef UNITY_SCOPES_INTERNAL_SETTINGSDB_H
#define UNITY_SCOPES_INTERNAL_SETTINGSDB_H

#include <unity/scopes/internal/Logger.h>
#include <unity/scopes/internal/SettingsSchema.h>
#include <unity/scopes/Variant.h>
#include <unity/util/DefinesPtrs.h>
#include <unity/util/NonCopyable.h>
#include <unity/util/ResourcePtr.h>

#include <sys/inotify.h>
#include <sys/ioctl.h>

#include <atomic>
#include <thread>

namespace unity
{

namespace util
{
class IniParser;
}

namespace scopes
{

namespace internal
{

class SettingsDB final
{
public:
    NONCOPYABLE(SettingsDB);
    UNITY_DEFINES_PTRS(SettingsDB);

    static UPtr create_from_ini_file(std::string const& db_path,
                                     std::string const& ini_file_path,
                                     boost::log::sources::severity_channel_logger_mt<>& logger);
    static UPtr create_from_json_string(std::string const& db_path,
                                        std::string const& json_string,
                                        boost::log::sources::severity_channel_logger_mt<>& logger);
    static UPtr create_from_schema(std::string const& db_path,
                                   unity::scopes::internal::SettingsSchema const& schema,
                                   boost::log::sources::severity_channel_logger_mt<>& logger);

    ~SettingsDB();

    SettingsDB(SettingsDB&&) = default;
    SettingsDB& operator=(SettingsDB&&) = default;

    VariantMap settings();  // Returns the current settings (checking the DB each time).

private:
    enum ThreadState
    {
        Idle,
        Running,
        Stopping,
        Failed
    };

    SettingsDB(std::string const& db_path,
               unity::scopes::internal::SettingsSchema const& schema,
               boost::log::sources::severity_channel_logger_mt<>& logger);

    void process_doc_(std::string const& id, unity::util::IniParser const& parer);
    void process_all_docs();
    void set_defaults();
    void watch_thread();

    bool state_changed_;
    std::string db_path_;
    unity::util::ResourcePtr<int, std::function<void(int)>> fd_;
    unity::util::ResourcePtr<int, std::function<void(int)>> watch_;
    VariantArray definitions_;                       // Returned by SettingsSchema
    std::map<std::string, Variant> def_map_;  // Allows fast access to the Variants in definitions_
    unity::scopes::VariantMap values_;
    std::thread thread_;
    std::mutex mutex_;
    ThreadState thread_state_;
    boost::log::sources::severity_channel_logger_mt<>& logger_;
};

}  // namespace internal

}  // namespace scopes

}  // namespace unity

#endif

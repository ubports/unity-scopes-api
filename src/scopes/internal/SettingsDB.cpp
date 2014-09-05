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
 * Authored by:
 *   Michi Henning <michi.henning@canonical.com>
 *   Pete Woods <pete.woods@canonical.com>
 */

#include <unity/scopes/internal/SettingsDB.h>

#include <unity/scopes/internal/IniSettingsSchema.h>
#include <unity/scopes/internal/JsonSettingsSchema.h>
#include <unity/UnityExceptions.h>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem.hpp>

#include <fcntl.h>

using namespace unity::util;
using namespace std;

namespace unity
{

namespace scopes
{

namespace internal
{

namespace
{

typedef ResourcePtr<int, function<void(int)>> FileLock;

static FileLock unix_lock(string const& path)
{
    FileLock file_lock(::open(path.c_str(), O_RDONLY), [](int fd)
    {
        if (fd != -1)
        {
            close(fd);
        }
    });

    if (file_lock.get() == -1)
    {
        throw FileException("Couldn't open file " + path, errno);
    }

    struct flock fl;
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 0;
    fl.l_type = F_RDLCK;

    if (::fcntl(file_lock.get(), F_SETLKW, &fl) != 0)
    {
        throw FileException("Couldn't get file lock for " + path, errno);
    }

    return file_lock;
}

static void watch_deleter(int fd, int watch)
{
    if (fd >= 0 && watch >= 0)
    {
        inotify_rm_watch(fd, watch);
    }
}

static const char* GROUP_NAME = "General";

}  // namespace

SettingsDB::UPtr SettingsDB::create_from_ini_file(string const& db_path, string const& ini_file_path)
{
    // Parse schema
    try
    {
        SettingsSchema::UPtr schema = IniSettingsSchema::create(ini_file_path);
        return create_from_schema(db_path, *schema);
    }
    catch (exception const& e)
    {
        throw ResourceException("SettingsDB::create_from_ini_file(): schema = " + ini_file_path + ", db = " + db_path);
    }
}

SettingsDB::UPtr SettingsDB::create_from_json_string(string const& db_path, string const& json_string)
{
    // Parse schema
    try
    {
        auto schema = JsonSettingsSchema::create(json_string);
        return create_from_schema(db_path, *schema);
    }
    catch (exception const& e)
    {
        throw ResourceException("SettingsDB::create_from_json_string(): cannot parse schema, db = " + db_path);
    }
}

SettingsDB::UPtr SettingsDB::create_from_schema(string const& db_path, SettingsSchema const& schema)
{
    return UPtr(new SettingsDB(db_path, move(schema)));
}

SettingsDB::SettingsDB(string const& db_path, SettingsSchema const& schema)
    : state_changed_(false)
    , db_path_(db_path)
    , fd_(inotify_init(), bind(&close, placeholders::_1))
    , watch_(-1, bind(&watch_deleter, fd_.get(), placeholders::_1))
    , thread_state_(Idle)
{
    // Validate the file descriptor
    if (fd_.get() < 0)
    {
        throw SyscallException("SettingsDB(): inotify_init() failed on inotify fd (fd = " +
                               to_string(fd_.get()) + ")", errno);
    }

    // Initialize the def_map_ so we can look things
    // up quickly.
    definitions_ = schema.definitions();
    for (auto const& d : definitions_)
    {
        def_map_.emplace(make_pair(d.get_dict()["id"].get_string(), d));
    }

    process_all_docs();
}


SettingsDB::~SettingsDB()
{
    // Tell the thread to stop politely
    {
        lock_guard<mutex> lock(mutex_);
        // Important to stop the watch, as this unblocks the select() call
        watch_.dealloc();
        thread_state_ = ThreadState::Stopping;
    }

    // Wait for thread to terminate
    if (thread_.joinable()) {
        thread_.join();
    }
}

void SettingsDB::watch_thread()
{
    try
    {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(fd_.get(), &fds);

        int bytes_avail = 0;
        string buffer;

        // Poll for notifications until stop is requested
        while (true)
        {
            // Wait for a payload to arrive
            int ret = select(fd_.get() + 1, &fds, nullptr, nullptr, nullptr);
            if (ret < 0)
            {
                throw SyscallException("SettingsDB::watch_thread(): Thread aborted: "
                                       "select() failed on inotify fd (fd = " +
                                       to_string(fd_.get()) + ")", errno);
            }

            // Get number of bytes available
            ret = ioctl(fd_.get(), FIONREAD, &bytes_avail);
            if (ret < 0)
            {
                throw SyscallException("SettingsDB::watch_thread(): Thread aborted: "
                                       "ioctl() failed on inotify fd (fd = " +
                                       to_string(fd_.get()) + ")", errno);
            }

            // Read available bytes
            buffer.resize(bytes_avail);
            int bytes_read = read(fd_.get(), &buffer[0], buffer.size());
            if (bytes_read < 0)
            {
                throw SyscallException("SettingsDB::watch_thread(): Thread aborted: "
                                       "read() failed on inotify fd (fd = " +
                                       to_string(fd_.get()) + ")", errno);
            }

            // Process event(s) received
            int i = 0;
            while (i < bytes_read)
            {
                struct inotify_event* event = reinterpret_cast<inotify_event*>(&buffer[i]);

                if (event->mask & IN_DELETE_SELF)
                {
                    lock_guard<mutex> lock(mutex_);
                    state_changed_ = false;
                    watch_.dealloc();
                    watch_.reset(-1);
                    thread_state_ = Stopping;
                }
                else if (event->mask & IN_CLOSE_WRITE)
                {
                    lock_guard<mutex> lock(mutex_);
                    state_changed_ = true;
                }
                i += sizeof(inotify_event) + event->len;
            }


            // Break from the loop if we are stopping
            {
                lock_guard<mutex> lock(mutex_);
                if (thread_state_ == Stopping)
                {
                    thread_state_ = Idle;
                    break;
                }
            }
        }
    }
    catch (exception const& e)
    {
        cerr << "SettingsDB::watch_thread(): Thread aborted: " << e.what() << endl;
        lock_guard<mutex> lock(mutex_);
        thread_state_ = Failed;
    }
    catch (...)
    {
        cerr << "SettingsDB::watch_thread(): Thread aborted: unknown exception" << endl;
        lock_guard<mutex> lock(mutex_);
        thread_state_ = Failed;
    }
}

// Called once for each setting. We are lenient when parsing
// the setting value. If it doesn't match the schema, or something goes
// wrong otherwise, we use the default value rather than complaining.
// It is up to the writer of the DB to ensure that the DB contents
// match the schema.

void SettingsDB::process_doc_(string const& id, IniParser const& p)
{
    auto def = def_map_.find(id);

    if (def == def_map_.end())
    {
        return;  // We ignore anything for which we don't have a schema.
    }


    string type = def->second.get_dict()["type"].get_string();
    try
    {
        if (type == "boolean")
        {
            values_[id] = Variant(p.get_boolean(GROUP_NAME, id));
        }
        else if (type == "list")
        {
            values_[id] = Variant(p.get_int(GROUP_NAME, id));
        }
        else if (type == "number")
        {
            string value = p.get_string(GROUP_NAME, id);
            try
            {
                values_[id] = Variant(stod(value));
            }
            catch (invalid_argument & e)
            {
            }
        }
        else if (type == "string")
        {
            values_[id] = Variant(p.get_string(GROUP_NAME, id));
        }
    }
    catch (LogicException & e)
    {
    }
}

void SettingsDB::process_all_docs()
{
    lock_guard<mutex> lock(mutex_);

    if (!watch_ || watch_.get() < 0)
    {
        boost::filesystem::path p(db_path_);
        if (boost::filesystem::exists(p))
        {
            watch_.reset(inotify_add_watch(fd_.get(),
                                           db_path_.c_str(),
                                           IN_CLOSE_WRITE
                                           | IN_DELETE_SELF));
            if (watch_.get() < 0)
            {
                throw ResourceException("SettingsDB::add_watch(): failed to add watch for path: \"" +
                                        db_path_ + "\". inotify_add_watch() failed. (fd = " +
                                        to_string(fd_.get()) + ", path = " + db_path_ + ")");
            }

            state_changed_ = true;

            if (thread_state_ == Idle)
            {
                thread_state_ = Running;
                thread_ = thread(&SettingsDB::watch_thread, this);
            }
        }
        else
        {
            state_changed_ = false;

            set_defaults();
            return;
        }
    }

    if (state_changed_)
    {
        state_changed_ = false;
        // We re-establish the defaults and re-read everything. We need to put the defaults back because
        // settings may have been deleted from the database.
        set_defaults();

        try
        {
            FileLock lock = unix_lock(db_path_);

            IniParser p(db_path_.c_str());

            if (p.has_group(GROUP_NAME))
            {
                auto keys = p.get_keys(GROUP_NAME);
                for (auto const& key : keys) {
                    process_doc_(key, p);
                }
            }
        }
        catch (FileException & e)
        {
            throw ResourceException(e.what());
        }
    }
}

void SettingsDB::set_defaults()
{
    values_.clear();
    for (auto const& f : definitions_)
    {
        auto def = f.get_dict();
        string const id = def["id"].get_string();
        Variant const default_value = def["defaultValue"];
        if (!default_value.is_null())
        {
            values_[id] = default_value;
        }
    }
}

VariantMap SettingsDB::settings()
{
    process_all_docs();
    return values_;
}

}  // namespace internal

}  // namespace scopes

}  // namespace unity

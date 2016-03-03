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
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

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
    catch (exception const&)
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
    catch (exception const&)
    {
        throw ResourceException("SettingsDB::create_from_json_string(): cannot parse schema, db = " + db_path);
    }
}

SettingsDB::UPtr SettingsDB::create_from_schema(string const& db_path, SettingsSchema const& schema)
{
    return UPtr(new SettingsDB(db_path, move(schema)));
}

SettingsDB::SettingsDB(string const& db_path, SettingsSchema const& schema)
    : db_path_(db_path)
    , last_write_time_nsec_(-1)
    , last_write_time_sec_(-1)
    , last_write_inode_(0)
{
    // Initialize the def_map_ so we can look things
    // up quickly.
    definitions_ = schema.definitions();
    for (auto const& d : definitions_)
    {
        def_map_.emplace(make_pair(d.get_dict()["id"].get_string(), d));
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
            catch (invalid_argument const&)
            {
            }
        }
        else if (type == "string")
        {
            values_[id] = Variant(p.get_string(GROUP_NAME, id));
        }
    }
    catch (LogicException const&)
    {
    }
}

void SettingsDB::process_all_docs()
{
    try
    {
        struct stat st;
        bool file_exists = ::stat(db_path_.c_str(), &st) == 0 && S_ISREG(st.st_mode);

        if (file_exists)
        {
            FileLock lock = unix_lock(db_path_);
            if (::fstat(lock.get(), &st) == 0) // re-stat the file
            {
                // it's neccessary to use both sec and nsec, cause it seems that on low-res kernels tv_nsec can duplicate from second to second.
                if (st.st_mtim.tv_nsec != last_write_time_nsec_ || st.st_mtim.tv_sec != last_write_time_sec_ || st.st_ino != last_write_inode_)
                {
                    // We re-establish the defaults and re-read everything. We need to put the defaults back because
                    // settings may have been deleted from the database.
                    set_defaults();

                    try
                    {
                        IniParser p(db_path_.c_str());

                        if (p.has_group(GROUP_NAME))
                        {
                            auto keys = p.get_keys(GROUP_NAME);
                            for (auto const& key : keys) {
                                process_doc_(key, p);
                            }
                        }
                    }
                    catch (FileException const& e)
                    {
                        if (e.error() == EACCES) // very unlikely; only if permissions changed after we acquired the lock
                        {
                            throw;
                        }
                        throw ResourceException(e.what());
                    }

                    last_write_time_nsec_ = st.st_mtim.tv_nsec;
                    last_write_time_sec_ = st.st_mtim.tv_sec;
                    last_write_inode_ = st.st_ino;

                    return;
                }
            }
        }
        else
        {
            set_defaults();
        }
    }
    catch (FileException const& e)
    {
        if (e.error() == EACCES)
        {
            throw;
        }

        // Failure in obtaining the lock shouldn't be reported to the scope, it's not fatal;
        // instead give the last known values (or defaults) back.
    }

    // Only set default values if we don't have some values already; we might have failed because
    // of a temporary issue and therefore we want to present most recent cached settings.
    if (values_.size() == 0)
    {
        set_defaults();
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

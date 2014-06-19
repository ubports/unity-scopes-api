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
 * Authored by Michi Henning <michi.henning@canonical.com>
 */

#include <unity/scopes/internal/SettingsDB.h>

#include <unity/UnityExceptions.h>

#include <boost/filesystem.hpp>

using namespace boost;
using namespace std;

namespace unity
{

namespace scopes
{

namespace internal
{

namespace
{

// Custom deleter for ResourcePtr db_ because u1db API requires a pointer
// to a pointer to close the database.

void free_db(u1database* p)
{
    u1db_free(&p);
}

}  // namespace

SettingsDB::SettingsDB(string const& path, string const& json_schema)
    : path_(path)
    , db_(nullptr, &free_db)
{
    // Parse schema
    try
    {
        schema_.reset(new SettingsSchema(json_schema));
    }
    catch (std::exception const& e)
    {
        throw ResourceException("SettingsDB(): cannot parse schema, db = " + path);
    }

    filesystem::path p(path);
    if (!filesystem::exists(p))
    {
        throw FileException("SettingsDB(): cannot open " + path, ENOENT);
    }
    db_.reset(u1db_open(p.native().c_str()));
    if (db_.get() == nullptr)
    {
        // We don't get any more detailed info from the u1db API.
        throw ResourceException("SettingsDB(): u1db_open() failed, db = " + path);
    }
}

SettingsDB::~SettingsDB() = default;

extern "C"
{

int callback(void* /* context */, u1db_document *doc)
{
    cerr << "got doc: " << doc->doc_id << endl;
    cerr << "json: " << doc->json << endl;
    u1db_free_doc(&doc);
    return 0;
}

}

void SettingsDB::get_all_docs()
{
    int generation;
    u1db_get_all_docs(db_.get(), 0, &generation, nullptr, callback);
}

} // namespace internal

} // namespace scopes

} // namespace unity

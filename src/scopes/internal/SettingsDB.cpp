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

#include <unity/scopes/internal/SettingsSchema.h>
#include <unity/UnityExceptions.h>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem.hpp>
#include <jsoncpp/json/json.h>

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

// Custom deleters for ResourcePtr because the u1db API requires
// a pointer to a pointer to release things.

auto db_deleter = [](u1database* p){ u1db_free(&p); };
auto doc_deleter = [](u1db_document* p){ u1db_free_doc(&p); };

}  // namespace

SettingsDB::SettingsDB(string const& path, string const& json_schema)
    : state_changed_(false)
    , path_(path)
    , db_(nullptr, db_deleter)
    , generation_(-1)
{
    // Parse schema
    try
    {
        SettingsSchema schema(json_schema);
        definitions_ = schema.definitions();
    }
    catch (std::exception const& e)
    {
        throw ResourceException("SettingsDB(): cannot parse schema, db = " + path);
    }

    process_all_docs();
}

SettingsDB::~SettingsDB() = default;

// Called once by u1db for each setting. We are lenient when parsing
// the setting value. If it doesn't match the schema, or something goes
// wrong otherwise, we use the default value rather than complaining.
// It is up to the writer of the DB to ensure that the DB contents
// match the schema.

void SettingsDB::process_doc_(string const& id, string const& json)
{
    // The shell writes the IDs with a prefix, so we need to strip that before
    // looking for the ID in the schema.
    static string const ID_PREFIX = "default-";
    if (!boost::starts_with(id, ID_PREFIX))
    {
        return;  // We ignore anything that doesn't match the prefix.
    }

    auto const it = definitions_.find(id.substr(ID_PREFIX.size()));
    if (it == definitions_.end())
    {
        return;  // We ignore anything for which we don't have a schema.
    }

    // Parse the value definition
    Json::Reader reader;
    Json::Value root;
    Json::Value val;
    if (!reader.parse(json, root) || !root.isObject() || (val = root["value"]).isNull())
    {
        // Broken database (syntax error or unexpected JSON objects).
        return;
    }

    // Convert the JSON value to a Variant of the correct type according to the schema.
    switch (val.type())
    {
        case Json::ValueType::stringValue:
        {
            if (val.isString())
            {
                values_[it->first] = Variant(val.asString());
            }
            break;
        }
        case Json::ValueType::booleanValue:
        {
            if (val.isBool())
            {
                values_[it->first] = Variant(val.asBool());
            }
            break;
        }
        case Json::ValueType::intValue:
        {
            if (val.isInt())
            {
                values_[it->first] = Variant(val.asInt());
            }
            break;
        }
        default:
        {
            // Ignore all other value types, because they don't make sense.
            break;
        }
    }
}

namespace
{

extern "C"
{

int check_for_changes(void* instance, char const* /* doc_id */, int /* gen*/, char const* /* trans_id */)
{
    SettingsDB* thisp = static_cast<SettingsDB*>(instance);
    assert(thisp);

    thisp->state_changed_ = true;
    return 0;
}

int process_docs(void* instance, u1db_document *docp)
{
    ResourcePtr<u1db_document*, decltype(doc_deleter)> doc(docp, doc_deleter);

    SettingsDB* thisp = static_cast<SettingsDB*>(instance);
    assert(thisp);

    thisp->process_doc_(doc.get()->doc_id, doc.get()->json);

    return 0;
}

}  // extern "C"

}  // namespace

void SettingsDB::process_all_docs()
{
    if (db_.get() == nullptr)
    {
        boost::filesystem::path p(path_);
        if (boost::filesystem::exists(p))
        {
            // Open the db. (It is possible that no DB existed when the constructor ran,
            // but the DB was created later, when the user first changed a setting.)
            db_.reset(u1db_open(path_.c_str()));
            if (db_.get() == nullptr)
            {
                // We don't get any more detailed info from the u1db API.
                throw ResourceException("SettingsDB: u1db_open() failed, db = " + path_);
            }
        }
        else
        {
            set_defaults();
            return;
        }
    }
    assert(db_.get());

    // Look for changed documents.
    // If there was an update, state_changed_ is set to true.
    state_changed_ = false;
    char* trans_id = nullptr;
    int new_generation = generation_;
    int status = u1db_whats_changed(db_.get(), &new_generation, &trans_id, this, check_for_changes);
    free(trans_id);  // We don't need it.
    if (status != U1DB_OK)
    {
        throw ResourceException("SettingsDB(): u1db_whats_changed() failed, status = "
                                + to_string(status) + ", db = " + path_);
    }

    if (state_changed_)
    {
        // We re-establish the defaults and re-read everything. We need to put the defaults back because
        // settings may have been deleted from the database.
        set_defaults();

        int new_generation = generation_;
        status = u1db_get_all_docs(db_.get(), 0, &new_generation, this, process_docs);
        if (status != U1DB_OK)
        {
            throw ResourceException("SettingsDB(): u1db_get_all_docs() failed, status = "
                                    + to_string(status) + ", db = " + path_);               // LCOV_EXCL_LINE
        }
        generation_ = new_generation;
    }
}

void SettingsDB::set_defaults()
{
    values_.clear();
    for (auto const& f : definitions_)
    {
        auto vmap = f.second.get_dict();
        auto v = vmap.find("defaultValue");
        if (v != vmap.end())
        {
            if (!v->second.is_null())
            {
                values_[f.first] = v->second;
            }
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

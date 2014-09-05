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

#include <unity/scopes/internal/JsonSettingsSchema.h>

#include <unity/UnityExceptions.h>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem.hpp>
#include <jsoncpp/json/json.h>

#include <set>

namespace unity
{

namespace scopes
{

namespace internal
{

using namespace unity;
using namespace unity::scopes;
using namespace boost;
using namespace boost::algorithm;
using namespace std;

namespace
{

// Parses out a single setting from the given JSON value and makes it available as a VariantMap.

class Setting final
{
public:
    NONCOPYABLE(Setting);

    Setting(Json::Value const& v);
    Setting(string const& id,
                string const& type,
                string const& display_name,
                VariantArray const& enumerators,
                Variant const& default_value);
    ~Setting() = default;

    string id() const;

    enum Type { BooleanT, ListT, NumberT, StringT };

    Variant to_schema_definition();

private:

    Json::Value get_mandatory(Json::Value const& v, Json::StaticString const& key, Json::ValueType expected_type) const;
    void set_default_value(Json::Value const& v, Type expected_type);
    Variant get_bool_default(Json::Value const& v) const;
    Variant get_enum_default(Json::Value const& v);
    Variant get_double_default(Json::Value const& v) const;
    Variant get_string_default(Json::Value const& v) const;
    void set_enumerators(Json::Value const& v);

    string id_;
    string type_;
    string display_name_;
    Variant default_value_;
    VariantArray enumerators_;
};

static const map<string, Setting::Type> VALID_TYPES {
                                                      { "boolean", Setting::BooleanT },
                                                      { "list",    Setting::ListT },
                                                      { "number",  Setting::NumberT },
                                                      { "string",  Setting::StringT },
                                                    };

Setting::Setting(Json::Value const& v)
{
    assert(v.isObject());

    static auto const id_key = Json::StaticString("id");
    static auto const type_key = Json::StaticString("type");
    static auto const display_name_key = Json::StaticString("displayName");

    id_ = get_mandatory(v, id_key, Json::stringValue).asString();
    if (id_.empty())
    {
        throw ResourceException(string("JsonSettingsSchema(): invalid empty \"") + id_key.c_str() + "\" definition");
    }

    auto v_type = get_mandatory(v, type_key, Json::stringValue);
    string type_string = v_type.asString();
    auto const it = VALID_TYPES.find(type_string);
    if (it == VALID_TYPES.end())
    {
        throw ResourceException(string("JsonSettingsSchema(): invalid \"") + type_key.c_str() + "\" setting: \""
                                       + type_string + "\", " "id = \"" + id_ + "\"");
    }
    type_ = it->first;
    set_default_value(v, it->second);

    display_name_ = get_mandatory(v, display_name_key, Json::stringValue).asString();
}

Setting::Setting(string const& id,
                 string const& type,
                 string const& display_name,
                 VariantArray const& enumerators,
                 Variant const& default_value)
    : id_(id)
    , display_name_(display_name)
    , default_value_(default_value)
    , enumerators_(enumerators)
{
    assert(!id.empty());

    auto const it = VALID_TYPES.find(type);
    if (it == VALID_TYPES.end())
    {
        throw ResourceException(string("JsonSettingsSchema(): invalid \"type\" definition: \"")
                                + type + "\", setting = \"" + id_ + "\"");
    }
    type_ = it->first;
}

string Setting::id() const
{
    return id_;
}

Variant Setting::to_schema_definition()
{
    VariantMap schema;
    schema["id"] = Variant(id_);
    schema["type"] = type_;
    schema["displayName"] = display_name_;
    schema["defaultValue"] = default_value_;
    if (type_ == "list")
    {
        schema["values"] = enumerators_;
    }
    return Variant(schema);
}

Json::Value Setting::get_mandatory(Json::Value const& v,
                                   Json::StaticString const& key,
                                   Json::ValueType expected_type) const
{
    assert(v.isObject());
    auto val = v[key];
    if (val.isNull())
    {
        throw ResourceException(string("JsonSettingsSchema(): missing \"") + key.c_str() + "\" definition"
                                       + (!id_.empty() ? ", id = \"" + id_ + "\"" : string()));
    }
    if (val.type() != expected_type)
    {
        throw ResourceException(string("JsonSettingsSchema(): invalid value type for \"") + key.c_str() + "\" definition, "
                                       + "id = \"" + id_ + "\"");
    }
    return val;
}

static auto const parameters_key = Json::StaticString("parameters");

void Setting::set_default_value(Json::Value const& v, Type expected_type)
{
    auto v_param = v[parameters_key];
    if (!v_param.isObject())
    {
        throw ResourceException("JsonSettingsSchema(): expected value of type object for \"parameters\", id = \"" + id_ + "\"");
    }
    switch (expected_type)
    {
        case Type::BooleanT:
        {
            default_value_ = get_bool_default(v_param);
            break;
        }
        case Type::ListT:
        {
            default_value_ = get_enum_default(v_param);
            break;
        }
        case Type::NumberT:
        {
            default_value_ = get_double_default(v_param);
            break;
        }
        case Type::StringT:
        {
            default_value_ = get_string_default(v_param);
            break;
        }
        default:
        {
            assert(false);  // LCOV_EXCL_LINE
        }
    }
}

static auto const dflt_key = Json::StaticString("defaultValue");

Variant Setting::get_bool_default(Json::Value const& v) const
{
    if (v.isNull())  // No "parameters" key
    {
        return Variant();
    }
    auto v_dflt = v[dflt_key];
    if (v_dflt.isNull())
    {
        return Variant();  // No "defaultValue" key
    }
    if (!v_dflt.isBool())
    {
        throw ResourceException(string("JsonSettingsSchema(): invalid value type for \"") + dflt_key.c_str()
                                       + "\" definition, id = \"" + id_ + "\"");
    }
    return Variant(v_dflt.asBool());
}

Variant Setting::get_double_default(Json::Value const& v) const
{
    if (v.isNull())  // No "parameters" key
    {
        return Variant();
    }
    auto v_dflt = v[dflt_key];
    if (v_dflt.isNull())
    {
        return Variant();  // No "defaultValue" key
    }
    if (!v_dflt.isNumeric())
    {
        throw ResourceException(string("JsonSettingsSchema(): invalid value type for \"") + dflt_key.c_str()
                                       + "\" definition, id = \"" + id_ + "\"");
    }
    return Variant(v_dflt.asDouble());
}

Variant Setting::get_enum_default(Json::Value const& v)
{
    assert(v.isObject());

    if (v.isNull())  // No "parameters" key
    {
        throw ResourceException(string("JsonSettingsSchema(): missing \"") + parameters_key.c_str()
                                       + "\" definition, id = \"" + id_ + "\"");
    }
    auto v_dflt = v[dflt_key];
    if (v_dflt.isNull())
    {
        return Variant();  // No "defaultValue" key
    }
    if (!v_dflt.isInt())
    {
        throw ResourceException(string("JsonSettingsSchema(): invalid value type for \"") + dflt_key.c_str()
                                       + "\" definition, id = \"" + id_ + "\"");
    }

    static auto const values_key = Json::StaticString("values");

    auto v_vals = v[values_key];
    if (!v_vals.isArray())
    {
        throw ResourceException(string("JsonSettingsSchema(): invalid value type for \"") + values_key.c_str()
                                       + "\" definition, id = \"" + id_ + "\"");
    }
    if (v_vals.size() < 1)
    {
        throw ResourceException(string("JsonSettingsSchema(): invalid empty \"") + values_key.c_str()
                                       + "\" definition, id = \"" + id_ + "\"");
    }

    set_enumerators(v_vals);

    auto val = v_dflt.asInt();
    if (val < 0 || val >= static_cast<int>(v_vals.size()))
    {
        throw ResourceException(string("JsonSettingsSchema(): \"") + dflt_key.c_str()
                                       + "\" out of range, id = \"" + id_ + "\"");
    }
    return Variant(v_dflt.asInt());
}

Variant Setting::get_string_default(Json::Value const& v) const
{
    if (v.isNull())  // No "parameters" key
    {
        return Variant();
    }
    auto v_dflt = v[dflt_key];
    if (v_dflt.isNull())
    {
        return Variant();  // No "defaultValue" key
    }
    if (!v_dflt.isString())
    {
        throw ResourceException(string("JsonSettingsSchema(): invalid value type for \"") + dflt_key.c_str()
                                       + "\" definition, id = \"" + id_ + "\"");
    }
    return Variant(v_dflt.asString());
}

void Setting::set_enumerators(Json::Value const& v)
{
    assert(v.isArray());

    set<string> enums_seen;

    for (unsigned i = 0; i < v.size(); ++i)
    {
        Json::Value enumerator = v[i];
        if (!enumerator.isString())
        {
            throw ResourceException(string("JsonSettingsSchema(): invalid enumerator type, id = \"") + id_ + "\"");
        }
        string enum_str = enumerator.asString();
        if (enum_str.empty())
        {
            throw ResourceException(string("JsonSettingsSchema(): invalid empty enumerator, id = \"") + id_ + "\"");
        }
        if (enums_seen.find(enum_str) != enums_seen.end())
        {
            throw ResourceException(string("JsonSettingsSchema(): duplicate enumerator \"") + enum_str
                                    + "\", id = \"" + id_ + "\"");
        }
        enums_seen.insert(enum_str);
        enumerators_.emplace_back(Variant(enum_str));
    }
}

}  // namespace

JsonSettingsSchema::UPtr JsonSettingsSchema::create(string const& json_string)
{
    return UPtr(new JsonSettingsSchema(json_string));
}

JsonSettingsSchema::UPtr JsonSettingsSchema::create_empty()
{
    return UPtr(new JsonSettingsSchema());
}

JsonSettingsSchema::JsonSettingsSchema(string const& json_string)
{
    try
    {
        Json::Value root;
        Json::Reader reader;
        if (!reader.parse(json_string, root))
        {
            throw ResourceException("JsonSettingsSchema(): cannot parse schema: " + reader.getFormattedErrorMessages());
        }

        if (!root.isArray())
        {
            root = root["settings"];
        }
        if (root.isNull())
        {
            throw ResourceException("JsonSettingsSchema(): missing \"settings\" definition");
        }
        if (!root.isArray())
        {
            throw ResourceException("JsonSettingsSchema(): value \"settings\" must be an array");
        }
        set<string> seen_settings;
        for (unsigned i = 0; i < root.size(); ++i)
        {
            Setting s(root[i]);
            if (starts_with(s.id(), "internal."))
            {
                throw ResourceException(string("JsonSettingsSchema(): invalid key \"") + s.id() + "\" prefixed with \"internal.\"");
            }
            if (seen_settings.find(s.id()) != seen_settings.end())
            {
                throw ResourceException("JsonSettingsSchema(): duplicate definition, id = \"" + s.id() + "\"");
            }
            definitions_.push_back(s.to_schema_definition());
            seen_settings.insert(s.id());
        }
    }
    catch (ResourceException const&)
    {
        throw;
    }
    // LCOV_EXCL_START
    catch (std::exception const& e)
    {
        throw ResourceException(string("JsonSettingsSchema(): unexpected error: ") + e.what());
    }
    // LCOV_EXCL_STOP
}

JsonSettingsSchema::JsonSettingsSchema()
{
}

void JsonSettingsSchema::add_location_setting()
{
    Setting s("internal.location", "boolean", "Enable location data", VariantArray(), Variant(true));
    definitions_.push_back(s.to_schema_definition());
}

JsonSettingsSchema::~JsonSettingsSchema() = default;

VariantArray JsonSettingsSchema::definitions() const
{
    return definitions_;
}

} // namespace internal

} // namespace scopes

} // namespace unity

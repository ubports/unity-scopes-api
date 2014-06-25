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

#include <unity/scopes/internal/SettingsSchema.h>

#include <unity/UnityExceptions.h>

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
using namespace std;

namespace
{

// Parses out a single setting from the given JSON value and makes it available as a VariantMap.
// TODO: flesh out doc in SettingsSchema.h

class Setting final
{
public:
    NONCOPYABLE(Setting);

    Setting(Json::Value const& v);
    ~Setting() = default;

    enum Type { BooleanT, ListT, NumberT, StringT };

    string id() const;
    Variant to_schema_definition();

private:
    Json::Value get_mandatory(Json::Value const& v, Json::StaticString const& key, Json::ValueType expected_type) const;
    void set_default_value(Json::Value const& v, Type expected_type);
    Variant get_bool_default(Json::Value const& v) const;
    Variant get_enum_default(Json::Value const& v);
    Variant get_int_default(Json::Value const& v) const;
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
        throw ResourceException(string("SettingSchema(): invalid empty \"") + id_key.c_str() + "\" definition");
    }

    auto v_type = get_mandatory(v, type_key, Json::stringValue);
    string type_string = v_type.asString();
    auto const it = VALID_TYPES.find(type_string);
    if (it == VALID_TYPES.end())
    {
        throw ResourceException(string("SettingsSchema(): invalid \"") + type_key.c_str() + "\" setting: \""
                                       + type_string + "\", " "id = \"" + id_ + "\"");
    }
    type_ = it->first;
    set_default_value(v, it->second);

    display_name_ = get_mandatory(v, display_name_key, Json::stringValue).asString();
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
    schema["displayName"] = display_name_;              // TODO: this needs localizing
    VariantMap params;
    if (type_ == "list")
    {
        params["values"] = enumerators_;
        params["valuesDisplayNames"] = enumerators_;    // TODO: this needs localizing and semantic check
    }
    params["defaultValue"] = default_value_;
    schema["parameters"] = params;
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
        throw ResourceException(string("SettingSchema(): missing \"") + key.c_str() + "\" definition"
                                       + (!id_.empty() ? ", id = \"" + id_ + "\"" : string()));
    }
    if (val.type() != expected_type)
    {
        throw ResourceException(string("SettingsSchema(): invalid value type for \"") + key.c_str() + "\" definition, "
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
        throw ResourceException("SettingsSchema(): expected value of type object for \"parameters\", id = \"" + id_ + "\"");
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
            default_value_ = get_int_default(v_param);
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
        throw ResourceException(string("SettingsSchema(): invalid value type for \"") + dflt_key.c_str()
                                       + "\" definition, id = \"" + id_ + "\"");
    }
    return Variant(v_dflt.asBool());
}

Variant Setting::get_int_default(Json::Value const& v) const
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
    if (!v_dflt.isInt())
    {
        throw ResourceException(string("SettingsSchema(): invalid value type for \"") + dflt_key.c_str()
                                       + "\" definition, id = \"" + id_ + "\"");
    }
    return Variant(v_dflt.asInt());
}

Variant Setting::get_enum_default(Json::Value const& v)
{
    assert(v.isObject());

    if (v.isNull())  // No "parameters" key
    {
        throw ResourceException(string("SettingsSchema(): missing \"") + parameters_key.c_str()
                                       + "\" definition, id = \"" + id_ + "\"");
    }
    auto v_dflt = v[dflt_key];
    if (v_dflt.isNull())
    {
        return Variant();  // No "defaultValue" key
    }
    if (!v_dflt.isInt())
    {
        throw ResourceException(string("SettingsSchema(): invalid value type for \"") + dflt_key.c_str()
                                       + "\" definition, id = \"" + id_ + "\"");
    }

    static auto const values_key = Json::StaticString("values");

    auto v_vals = v[values_key];
    if (!v_vals.isArray())
    {
        throw ResourceException(string("SettingsSchema(): invalid value type for \"") + values_key.c_str()
                                       + "\" definition, id = \"" + id_ + "\"");
    }
    if (v_vals.size() < 1)
    {
        throw ResourceException(string("SettingsSchema(): invalid empty \"") + values_key.c_str()
                                       + "\" definition, id = \"" + id_ + "\"");
    }

    set_enumerators(v_vals);

    auto val = v_dflt.asInt();
    if (val < 0 || val >= static_cast<int>(v_vals.size()))
    {
        throw ResourceException(string("SettingsSchema(): \"") + dflt_key.c_str()
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
        throw ResourceException(string("SettingsSchema(): invalid value type for \"") + dflt_key.c_str()
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
            throw ResourceException(string("SettingsSchema(): invalid enumerator type, id = \"") + id_ + "\"");
        }
        string enum_str = enumerator.asString();
        if (enum_str.empty())
        {
            throw ResourceException(string("SettingsSchema(): invalid empty enumerator, id = \"") + id_ + "\"");
        }
        if (enums_seen.find(enum_str) != enums_seen.end())
        {
            throw ResourceException(string("SettingsSchema(): duplicate enumerator \"") + enum_str
                                    + "\", id = \"" + id_ + "\"");
        }
        enums_seen.insert(enum_str);
        enumerators_.emplace_back(Variant(enum_str));
    }
}

}  // namespace

SettingsSchema::SettingsSchema(string const& json_schema)
{
    try
    {
        Json::Value root;
        Json::Reader reader;
        if (!reader.parse(json_schema, root))
        {
            throw ResourceException("SettingsSchema(): cannot parse schema: " + reader.getFormattedErrorMessages());
        }

        auto settings = root["settings"];
        if (settings.isNull())
        {
            throw ResourceException("SettingsSchema(): missing \"settings\" definition");
        }
        if (!settings.isArray())
        {
            throw ResourceException("SettingsSchema(): value \"settings\" must be an array");
        }
        for (unsigned i = 0; i < settings.size(); ++i)
        {
            Setting s(settings[i]);
            definitions_.push_back(s.to_schema_definition());
        }
    }
    catch (ResourceException const&)
    {
        throw;
    }
    // LCOV_EXCL_START
    catch (std::exception const& e)
    {
        throw ResourceException(string("SettingsSchema(): unexpected error: ") + e.what());
    }
    // LCOV_EXCL_STOP
}

SettingsSchema::~SettingsSchema() = default;

VariantArray SettingsSchema::definitions() const
{
    return definitions_;
}

} // namespace internal

} // namespace scopes

} // namespace unity

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

#include <unity/scopes/internal/DfltConfig.h>
#include <unity/scopes/internal/JsonCppNode.h>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem.hpp>
#include <unity/UnityExceptions.h>

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

    Setting(Variant const& v);
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

    Variant get_mandatory(Variant const& v, string const& key, Variant::Type expected_type) const;
    void set_default_value(Variant const& v, Type expected_type);
    Variant get_bool_default(Variant const& v) const;
    Variant get_enum_default(Variant const& v);
    Variant get_double_default(Variant const& v) const;
    Variant get_string_default(Variant const& v) const;
    void set_enumerators(Variant const& v);

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

Setting::Setting(Variant const& v)
{
    assert(v.which() == Variant::Type::Dict);

    static string const id_key = "id";
    static string const type_key = "type";
    static string const display_name_key = "displayName";

    id_ = get_mandatory(v, id_key, Variant::Type::String).get_string();
    if (id_.empty())
    {
        throw ResourceException(string("JsonSettingsSchema(): invalid empty \"") + id_key + "\" definition");
    }

    auto v_type = get_mandatory(v, type_key, Variant::Type::String);
    string type_string = v_type.get_string();
    auto const it = VALID_TYPES.find(type_string);
    if (it == VALID_TYPES.end())
    {
        throw ResourceException(string("JsonSettingsSchema(): invalid \"") + type_key + "\" definition: \""
                                + type_string + "\", " "id = \"" + id_ + "\"");
    }
    type_ = it->first;
    set_default_value(v, it->second);

    display_name_ = get_mandatory(v, display_name_key, Variant::Type::String).get_string();
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
    assert(it != VALID_TYPES.end());
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
        schema["displayValues"] = enumerators_;
    }
    return Variant(schema);
}

Variant Setting::get_mandatory(Variant const& v, string const& key, Variant::Type expected_type) const
{
    assert(v.which() == Variant::Type::Dict);
    auto val = v.get_dict()[key];
    if (val.is_null())
    {
        throw ResourceException(string("JsonSettingsSchema(): missing \"") + key + "\" definition"
                                + (!id_.empty() ? ", id = \"" + id_ + "\"" : string()));
    }
    if (val.which() != expected_type)
    {
        throw ResourceException(string("JsonSettingsSchema(): invalid value type for \"") + key
                                + "\" definition, " + "id = \"" + id_ + "\"");
    }
    return val;
}

static string const parameters_key = "parameters";

void Setting::set_default_value(Variant const& v, Type expected_type)
{
    auto v_param = v.get_dict()[parameters_key];
    if (v_param.is_null())
    {
        return;
    }
    else if (v_param.which() != Variant::Type::Dict)
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

static string const dflt_key = "defaultValue";

Variant Setting::get_bool_default(Variant const& v) const
{
    assert(!v.is_null());

    auto v_dflt = v.get_dict()[dflt_key];
    if (v_dflt.is_null())
    {
        return Variant();  // No "defaultValue" key
    }
    if (v_dflt.which() != Variant::Type::Bool)
    {
        throw ResourceException(string("JsonSettingsSchema(): invalid value type for \"") + dflt_key
                                + "\" definition, id = \"" + id_ + "\"");
    }
    return v_dflt;
}

Variant Setting::get_double_default(Variant const& v) const
{
    assert(!v.is_null());

    auto v_dflt = v.get_dict()[dflt_key];
    if (v_dflt.is_null())
    {
        return Variant();  // No "defaultValue" key
    }
    switch (v_dflt.which())
    {
        case Variant::Type::Int:
            return Variant(double(v_dflt.get_int()));
        case Variant::Type::Int64:
            return Variant(double(v_dflt.get_int64_t()));
        case Variant::Type::Double:
            return v_dflt;
        default:
            throw ResourceException(string("JsonSettingsSchema(): invalid value type for \"") + dflt_key
                                    + "\" definition, id = \"" + id_ + "\"");
    }
}

Variant Setting::get_enum_default(Variant const& v)
{
    assert(v.which() == Variant::Type::Dict);

    auto v_dflt = v.get_dict()[dflt_key];
    if (v_dflt.is_null())
    {
        return Variant();  // No "defaultValue" key
    }
    if (v_dflt.which() != Variant::Type::Int)
    {
        throw ResourceException(string("JsonSettingsSchema(): invalid value type for \"") + dflt_key
                                + "\" definition, id = \"" + id_ + "\"");
    }

    static string const values_key = "values";

    auto v_vals = v.get_dict()[values_key];
    if (v_vals.which() != Variant::Type::Array)
    {
        throw ResourceException(string("JsonSettingsSchema(): invalid value type for \"") + values_key
                                + "\" definition, id = \"" + id_ + "\"");
    }
    if (v_vals.get_array().size() == 0)
    {
        throw ResourceException(string("JsonSettingsSchema(): invalid empty \"") + values_key
                                + "\" definition, id = \"" + id_ + "\"");
    }

    set_enumerators(v_vals);

    auto val = v_dflt.get_int();
    if (val < 0 || val >= static_cast<int>(v_vals.get_array().size()))
    {
        throw ResourceException(string("JsonSettingsSchema(): \"") + dflt_key
                                + "\" out of range, id = \"" + id_ + "\"");
    }
    return v_dflt;
}

Variant Setting::get_string_default(Variant const& v) const
{
    assert(!v.is_null());

    auto v_dflt = v.get_dict()[dflt_key];
    if (v_dflt.is_null())
    {
        return Variant();  // No "defaultValue" key
    }
    if (v_dflt.which() != Variant::Type::String)
    {
        throw ResourceException(string("JsonSettingsSchema(): invalid value type for \"") + dflt_key
                                + "\" definition, id = \"" + id_ + "\"");
    }
    return v_dflt;
}

void Setting::set_enumerators(Variant const& v)
{
    assert(v.which() == Variant::Type::Array);

    set<string> enums_seen;

    VariantArray va = v.get_array();
    for (unsigned i = 0; i < va.size(); ++i)
    {
        Variant enumerator = v.get_array()[i];
        if (enumerator.which() != Variant::Type::String)
        {
            throw ResourceException(string("JsonSettingsSchema(): invalid enumerator type, id = \"") + id_ + "\"");
        }
        string enum_str = enumerator.get_string();
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
        enumerators_.emplace_back(enumerator);
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
        JsonCppNode::SPtr root = make_shared<JsonCppNode>(json_string);

        if (root->type() != JsonNodeInterface::Array)
        {
            if (root->type() != JsonNodeInterface::Object || !root->has_node("settings"))
            {
                throw ResourceException("JsonSettingsSchema(): missing \"settings\" definition");
            }
            root = root->get_node("settings");
            if (root->type() != JsonNodeInterface::Array)
            {
                throw ResourceException("JsonSettingsSchema(): value \"settings\" must be an array");
            }
        }

        set<string> seen_settings;
        for (int i = 0; i < root->size(); ++i)
        {
            Setting s(root->get_node(i)->to_variant());
            if (starts_with(s.id(), "internal."))
            {
                throw ResourceException(string("JsonSettingsSchema(): invalid key \"") + s.id()
                                        + "\" prefixed with \"internal.\"");
            }
            if (seen_settings.find(s.id()) != seen_settings.end())
            {
                throw ResourceException("JsonSettingsSchema(): duplicate definition, id = \"" + s.id() + "\"");
            }
            seen_settings.insert(s.id());
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
        throw ResourceException(string("JsonSettingsSchema(): unexpected error: ") + e.what());
    }
    // LCOV_EXCL_STOP
}

JsonSettingsSchema::JsonSettingsSchema()
{
}

void JsonSettingsSchema::add_location_setting()
{
    // TODO: HACK: See bug #1393438.
    //             Temporarily work around this problem by adding an entry to each scope's settings
    //             schema with the location data boolean. The shell
    //             intercepts this as a "special" setting and takes care of translating the
    //             display string. Eventually, we'll need to fix this, removing this hack.
    //             Realistically, the shell should not store this user-preference
    //             in the scope's settings database, and should only pay attention to the scope's
    //             LocationDataNeeded metadata attribute.
    Setting s("internal.location", "boolean", "Enable location data", VariantArray(), Variant(DFLT_LOCATION_PERMITTED));
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

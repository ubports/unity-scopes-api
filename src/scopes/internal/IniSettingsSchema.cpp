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

#include <unity/scopes/internal/IniSettingsSchema.h>

#include <unity/UnityExceptions.h>

#include <boost/algorithm/string/predicate.hpp>

#include <cassert>

namespace unity
{

namespace scopes
{

namespace internal
{

using namespace unity;
using namespace unity::scopes;
using namespace unity::util;
using namespace std;
using namespace boost::algorithm;

namespace
{

class Setting final
{
public:
    NONCOPYABLE(Setting);

    Setting(shared_ptr<IniParser> const& p, string const& id);
    Setting(string const& id,
            string const& type,
            string const& display_name,
            VariantArray const& display_values,
            Variant const& default_value);
    ~Setting() = default;

    string id() const;

    enum Type { BooleanT, ListT, NumberT, StringT };

    Variant to_schema_definition();

private:
    static constexpr auto id_key = "id";
    static constexpr auto type_key = "type";
    static constexpr auto dflt_val_key = "defaultValue";
    static constexpr auto d_name_key = "displayName";
    static constexpr auto d_values_key = "displayValues";

    string get_mandatory_string(string const& key);
    string get_mandatory_localized_string(string const& key);
    vector<string> get_mandatory_localized_string_array(string const& key);
    void set_value(Type expected_type);

    shared_ptr<IniParser> p_;
    string id_;
    string type_;
    string display_name_;
    VariantArray display_values_;
    Variant default_value_;
};

static const map<string, Setting::Type> VALID_TYPES {
                                                      { "boolean", Setting::BooleanT },
                                                      { "list",    Setting::ListT },
                                                      { "number",  Setting::NumberT },
                                                      { "string",  Setting::StringT },
                                                    };

Setting::Setting(shared_ptr<IniParser> const& p, string const& id)
    : p_(p)
    , id_(id)
{
    assert(!id.empty());

    string type = get_mandatory_string(type_key);
    auto const it = VALID_TYPES.find(type);
    if (it == VALID_TYPES.end())
    {
        throw ResourceException(string("IniSettingsSchema(): invalid \"") + type_key + "\" definition: \""
                                + type + "\", setting = \"" + id_ + "\"");
    }
    type_ = it->first;

    set_value(it->second);

    display_name_ = get_mandatory_localized_string(d_name_key);
}

Setting::Setting(string const& id,
                 string const& type,
                 string const& display_name,
                 VariantArray const& display_values,
                 Variant const& default_value)
    : id_(id)
    , display_name_(display_name)
    , display_values_(display_values)
    , default_value_(default_value)
{
    assert(!id.empty());

    auto const it = VALID_TYPES.find(type);
    if (it == VALID_TYPES.end())
    {
        throw ResourceException(string("IniSettingsSchema(): invalid \"") + type_key + "\" definition: \""
                                + type + "\", setting = \"" + id_ + "\"");
    }
    type_ = it->first;
}


Variant Setting::to_schema_definition()
{
    VariantMap schema;
    schema[id_key] = Variant(id_);
    schema[type_key] = type_;
    schema[d_name_key] = display_name_;
    schema[dflt_val_key] = default_value_;
    if (type_ == "list")
    {
        schema[d_values_key] = Variant(display_values_);
    }
    return Variant(schema);
}

string Setting::get_mandatory_string(string const& key)
{
    try
    {
        return p_->get_string(id_, key);
    }
    catch (LogicException const&)
    {
        throw ResourceException("IniSettingsSchema(): missing \"" + key + "\" definition, setting = \"" + id_ + "\"");
    }
}

string Setting::get_mandatory_localized_string(string const& key)
{
    try
    {
        return p_->get_locale_string(id_, key);
    }
    catch (LogicException const&)
    {
        throw ResourceException("IniSettingsSchema(): missing \"" + key + "\" definition, setting = \"" + id_ + "\"");
    }
}

vector<string> Setting::get_mandatory_localized_string_array(string const& key)
{
    try
    {
        return p_->get_locale_string_array(id_, key);
    }
    catch (LogicException const&)
    {
        throw ResourceException("IniSettingsSchema(): missing \"" + key + "\" definition, setting = \"" + id_ + "\"");
    }
}

void Setting::set_value(Type expected_type)
{
    if (!p_->has_key(id_, dflt_val_key))
    {
        return;  // No default value set.
    }

    try
    {
        switch (expected_type)
        {
            case Setting::BooleanT:
            {
                default_value_ = Variant(p_->get_boolean(id_, dflt_val_key));
                break;
            }
            case Setting::ListT:
            {
                default_value_ = Variant(p_->get_int(id_, dflt_val_key));
                auto values = get_mandatory_localized_string_array(d_values_key);
                if (values.size() < 2)
                {
                    throw ResourceException(string("IniSettingsSchema(): invalid number of entries for \"") + d_values_key +
                                            "\" definition, setting = \"" + id_ + "\"");
                }
                if (default_value_.get_int() < 0 || default_value_.get_int() >= int(values.size()))
                {
                    throw ResourceException(string("IniSettingsSchema(): \"") + d_values_key + "\" out of range, "
                                            "setting = \"" + id_ + "\"");
                }
                for (auto const& v : values)
                {
                    display_values_.push_back(Variant(v));
                }
                break;
            }
            case Setting::NumberT:
            {
                default_value_ = Variant(p_->get_int(id_, dflt_val_key));
                break;
            }
            case Setting::StringT:
            {
                default_value_ = Variant(p_->get_string(id_, dflt_val_key));
                break;
            }
            default:
            {
                assert(false);  // LCOV_EXCL_LINE
            }
        }
    }
    catch (LogicException const& e)
    {
        throw ResourceException(string("IniSettingsSchema(): invalid value type for \"") + dflt_val_key + "\" definition, "
                                "setting = \"" + id_ + "\"");
    }
}

}  // namespace

IniSettingsSchema::UPtr IniSettingsSchema::create(string const& ini_file)
{
    return UPtr(new IniSettingsSchema(ini_file));
}

IniSettingsSchema::UPtr IniSettingsSchema::create_empty()
{
    return UPtr(new IniSettingsSchema());
}

IniSettingsSchema::IniSettingsSchema(string const& ini_file)
    : ini_file_(ini_file)
{
    if (ini_file.empty())
    {
        return;
    }

    try
    {
        auto p = make_shared<IniParser>(ini_file.c_str());

        auto settings = p->get_groups();
        for (auto const& id: settings)
        {
            if(starts_with(id, "internal:")) {
                throw ResourceException(string("IniSettingsSchema(): invalid key \"") + id + "\" prefixed with \"internal:\"");
            }
            Setting s(p, id);
            definitions_.push_back(s.to_schema_definition());
        }
    }
    catch (std::exception const& e)
    {
        throw ResourceException("IniSettingsSchema(): cannot parse settings file \"" + ini_file + "\"");
    }
}

IniSettingsSchema::~IniSettingsSchema() = default;

VariantArray IniSettingsSchema::definitions() const
{
    return definitions_;
}

void IniSettingsSchema::add_location_setting()
{
    Setting s("internal:location", "boolean", "Enable location data", VariantArray(), Variant(true));
    definitions_.push_back(s.to_schema_definition());
}

} // namespace internal

} // namespace scopes

} // namespace unity

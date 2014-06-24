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

#include <unity/scopes/internal/SettingsSchema.h>
#include <unity/util/ResourcePtr.h>

#include <u1db/u1db.h>

namespace unity
{

namespace scopes
{

namespace internal
{

class SettingsDB final
{
public:
    NONCOPYABLE(SettingsDB);
    UNITY_DEFINES_PTRS(SettingsDB);

    SettingsDB(std::string const& path, std::string const& json_schema);
    ~SettingsDB();

    SettingsDB(SettingsDB&&) = default;
    SettingsDB& operator=(SettingsDB&&) = default;

    VariantMap settings();  // Returns the current settings (checking the DB each time).

    // These should be private, but we can't make them private because an extern "C"
    // callback cannot access private members.

    void process_doc_(std::string const& id, std::string const& json);

    bool state_changed_;

private:
    void process_all_docs();
    void set_defaults();

    std::string path_;
    unity::util::ResourcePtr<u1database*, std::function<void(u1database*)>> db_;
    int generation_;
    SettingsSchema::DefinitionMap definitions_;
    unity::scopes::VariantMap values_;
};

}  // namespace internal

}  // namespace scopes

}  // namespace unity

#endif

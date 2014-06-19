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

    void get_all_docs();

private:
    std::string path_;
    unity::scopes::internal::SettingsSchema::UPtr schema_;
    unity::util::ResourcePtr<u1database*, std::function<void(u1database*)>> db_;
};

} // namespace internal

} // namespace scopes

} // namespace unity

#endif

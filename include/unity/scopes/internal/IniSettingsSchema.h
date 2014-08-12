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

#ifndef UNITY_SCOPES_INTERNAL_INISETTINGSSCHEMA_H
#define UNITY_SCOPES_INTERNAL_INISETTINGSSCHEMA_H

#include <unity/scopes/internal/SettingsSchema.h>

namespace unity
{

namespace scopes
{

namespace internal
{

class IniSettingsSchema : public SettingsSchema
{
public:
    NONCOPYABLE(IniSettingsSchema);
    UNITY_DEFINES_PTRS(IniSettingsSchema);

    static UPtr create(std::string const& ini_file);

    static UPtr create_empty();

    ~IniSettingsSchema();

    IniSettingsSchema(IniSettingsSchema&&) = default;
    IniSettingsSchema& operator=(IniSettingsSchema&&) = default;

    virtual VariantArray definitions() const;

    virtual void add_location_setting();

private:
    IniSettingsSchema(std::string const& ini_file = std::string());

    std::string const ini_file_;
    VariantArray definitions_;
};

} // namespace internal

} // namespace scopes

} // namespace unity

#endif
